"""Fully connected layer"""

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

    def has_weights(self):
        """If the layer has weights"""
        return True

    def get_buffer_size(self):
        """If the layer needs extra buffer. Return the buffer shape tuple"""
        return None

    def get_code(self, jinja_dir, opt_config, qf):
        """Get the code for the layer"""
        path = os.path.join(jinja_dir, "fc.jinja")

        has_adaptive_gen_mem = False
        if opt_config["adaptive_gen_mem"]:
            has_adaptive_gen_mem = (has_adaptive_gen_mem or
                self.output_size[1] < opt_config["adaptive_gen_mem_size"]
            )

        if opt_config["global_mem_buffer"]:
            lea_src_size = math.floor((opt_config["lea_size"] - 2) / 2)
            lea_tmp_size = math.floor((opt_config["lea_size"] - 2) / 2)
        else:
            lea_src_size = opt_config["lea_size"]
            lea_tmp_size = opt_config["lea_size"]

        # Make sure all lea buffers are multiple of 2
        lea_src_size += (lea_src_size % 2)
        lea_tmp_size += (lea_tmp_size % 2)

        io_qf, weight_qf = qf

        params = {
            "layer_name" : self.name,
            "input_size" : self.input_size[1],
            "output_size" : self.output_size[1],
            "qf" : weight_qf,
            "lea_opt" : opt_config["lea_opt"],
            "lea_src_size" : lea_src_size,
            "lea_tmp_size" : lea_tmp_size,
            "has_loop_cpy" : True,
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
