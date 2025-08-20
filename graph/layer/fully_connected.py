# SPDX-License-Identifier: AGPL-3.0-or-later
# Copyright (C) 2025 Mingyuan Xiang

import os
import math

from jinja2 import Template

from .layer import LupeLayer
from .layer_utils import get_onnx_attr, name_conversion

class FullyConnected(LupeLayer):
    """FullyConnected layer
    
    A' = transpose(A) if transA else A
    B' = transpose(B) if transB else B
    Compute Y = alpha * A' * B' + beta * C
    https://onnx.ai/onnx/operators/onnx__Gemm.html
    
    """
    def _register(self, node):
        """Register the layer"""
        self.input = name_conversion(node.input[0])[:-len("_output_0")]
        # alpha
        alpha = get_onnx_attr(node, "alpha")
        self.alpha = alpha.f if alpha else None
        # beta
        beta = get_onnx_attr(node, "beta")
        self.beta = beta.f if beta else None
        # transA
        trans_a = get_onnx_attr(node, "transA")
        self.trans_a = trans_a is None or trans_a.i == 0
        # transB
        trans_b = get_onnx_attr(node, "transB")
        self.trans_b = trans_b is None or trans_b.i == 0

        self._acceleration = None
        self._calibration_list_idx = 0

    def __str__(self):
        s = f"{self.name}: FullyConnected("
        s += f"input={self.input}, "
        if self.alpha:
            s += f"alpha={self.alpha}, "
        if self.beta:
            s += f"beta={self.beta}, "
        s += f"transA={self.trans_a}, "
        s += f"transB={self.trans_b}"
        s += ")"

        return s

    def _get_name(self, node):
        """Get the name of the layer"""
        return name_conversion(node.name)

    def transpose(self):
        """If the layer should transpose the weights"""
        if self._get_acceleration() == "vec_mat":
            return True

        return False

    def _get_acceleration(self):
        if self._acceleration is None:
            return "mac"

        return self._acceleration

    def has_weights(self):
        """If the layer has weights"""
        return True

    def get_buffer_size(self, acceleration):
        """If the layer needs extra buffer. Return the buffer shape tuple"""
        return None

    def get_calibration(self):
        """Get the list of acceleration method for calibration"""
        calibration_list = ["mac", "vec_mat"]

        if self._calibration_list_idx >= len(calibration_list):
            return None
        else:
            return calibration_list[self._calibration_list_idx]

    def next_calibration_list_idx(self):
        """Increment _calibration_list_idx"""
        self._calibration_list_idx += 1

    def set_acceleration(self, acceleration):
        """Set which operation to use"""
        self._acceleration = acceleration

    def _get_size(self, opt_config, acceleration):
        mul = 16

        def _size_converter(x):
            """Convert the size to multiple of 16"""
            if x <= mul:
                return mul

            return math.ceil(x / mul) * mul

        out_size = self.output_size[1]

        # Make sure all lea buffers are multiple of 2
        if opt_config["global_mem_buffer"]:
            if acceleration == "mac":
                # 2 * size will always be smaller than opt_config["lea_size"]
                size = math.floor(opt_config["lea_size"] / 2) - mul
                size = _size_converter(size)
                lea_src_size = size
                lea_tmp_size = size
                lea_dst_size = 0 # no need for lea_dst buffer
            else: # vec_mat
                if out_size % 2:
                    out_size += 1

                if opt_config["lea_size"] < 3 * out_size + 1:
                    raise ValueError("LEA array size not big enough")

                s = opt_config["lea_size"] - out_size
                # (out_size + 1) * size will always be smaller than s
                size = math.floor(s / (out_size + 1))
                if size % 2:
                    size -= 1
                lea_src_size = size
                lea_tmp_size = size * out_size
                lea_dst_size = out_size
        else:
            lea_src_size = opt_config["lea_size"]
            lea_tmp_size = opt_config["lea_size"]
            lea_dst_size = opt_config["lea_size"]

        if out_size % 2:
            out_size += 1

        if acceleration == "vec_mat":
            if (lea_dst_size < out_size) or (lea_tmp_size < 2 * out_size):
                raise ValueError("LEA array size not big enough")

        return lea_src_size, lea_tmp_size, lea_dst_size

    def get_code(self, name, jinja_dir, opt_config, qf, acceleration):
        """Get the code for the layer"""
        if acceleration is None:
            acceleration = self._get_acceleration()

        if name is None:
            name = self.name

        path = os.path.join(jinja_dir, "fc")
        path = os.path.join(path, acceleration + ".jinja")

        sizes = self._get_size(opt_config, acceleration)
        lea_src_size, lea_tmp_size, lea_dst_size = sizes

        io_qf, weight_qf = qf

        if self.output_size[1] % 2:
            output_size_aligned = self.output_size[1] + 1
        else:
            output_size_aligned = self.output_size[1]

        if self.input_size[1] * output_size_aligned >= lea_tmp_size:
            mat_block_size = lea_tmp_size
        else:
            mat_block_size = self.input_size[1] * output_size_aligned

        mat_block_row_size = mat_block_size // output_size_aligned
        if mat_block_row_size % 2:
            mat_block_row_size -= 1

        has_adaptive_gen_mem = False
        if opt_config["adaptive_gen_mem"]:
            has_adaptive_gen_mem = (has_adaptive_gen_mem or
                self.output_size[1] < opt_config["adaptive_gen_mem_size"]
            )

            has_adaptive_gen_mem = (has_adaptive_gen_mem or
                (self.input_size[1] % mat_block_row_size) <
                opt_config["adaptive_gen_mem_size"]
            )

        params = {
            "layer_name" : name,
            "input_size" : self.input_size[1],
            "output_size" : self.output_size[1],
            "output_size_aligned" : output_size_aligned,
            "mat_size" : self.input_size[1] * self.output_size[1],
            "mat_block_size" : mat_block_size,
            "mat_block_row_size" : mat_block_row_size,
            "qf" : weight_qf,
            "lea_opt" : opt_config["lea_opt"],
            "lea_src_size" : lea_src_size,
            "lea_tmp_size" : lea_tmp_size,
            "lea_dst_size" : lea_dst_size,
            "has_adaptive_gen_mem" : has_adaptive_gen_mem,
            "global_mem_buffer" : opt_config["global_mem_buffer"],
        }

        if opt_config["adaptive_gen_mem"]:
            params["adaptive_gen_mem_size"] = (
                opt_config["adaptive_gen_mem_size"]
            )

        with open(path, "r", encoding="utf-8") as file:
            template = file.read()
            j_template = Template(template)
            code_str = j_template.render(params)

            return code_str
