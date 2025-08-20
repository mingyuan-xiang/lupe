# SPDX-License-Identifier: AGPL-3.0-or-later
# Copyright (C) 2025 Mingyuan Xiang

import os
import math

from jinja2 import Template

from .layer import LupeLayer
from .layer_utils import get_onnx_attr, name_conversion
from .helpers import get_stride

class Convolution2D(LupeLayer):
    """2D Convolution layer"""
    def _register(self, node):
        """Register the layer"""
        self.input = name_conversion(node.input[0])[:-len("_output_0")]
        # group
        group = get_onnx_attr(node, "group")
        self.group = group.i if group else None
        # kernel_shape
        kernel_shape = get_onnx_attr(node, "kernel_shape")
        self.kernel_shape = tuple(
            (self.output_size[1], self.input_size[1], *kernel_shape.ints)
        ) if kernel_shape else None
        # pads
        pads = get_onnx_attr(node, "pads")
        self.padding = tuple(pads.ints) if pads else (0, 0, 0, 0)
        # strides
        strides = get_onnx_attr(node, "strides")
        self.strides = tuple(strides.ints) if strides else None

        # Resize the input based on the padding.
        # The output size should be correct because onnx records
        # the correct size.
        # The order of the padding tuple is (left, right, top, bottom)
        if sum(self.padding) > 0:
            self.input_size = (
                self.input_size[0],
                self.input_size[1],
                self.input_size[2] + self.padding[2] + self.padding[3],
                self.input_size[3] + self.padding[0] + self.padding[1],
            )
            self.has_padding = True
        else:
            self.has_padding = False

        self._acceleration = None
        self._calibration_list_idx = 0

    def __str__(self):
        s = f"{self.name}: Convolution2D("
        s += f"input={self.input}, "
        s += f"kernel_shape={self.kernel_shape}, "
        s += f"padding={self.padding}, "
        s += f"strides={self.strides}"
        if self.group:
            s += f", group={self.group}"
        s += ")"

        return s

    def _get_name(self, node):
        """Get the name of the layer"""
        return name_conversion(node.name)

    def has_weights(self):
        """If the layer has weights"""
        return True

    def _get_acceleration(self):
        if self._acceleration is None:
            return "fir"
        else:
            if self.group != 1 and self._acceleration == "enhanced_mac":
                raise NotImplementedError(
                    "Depth-wise convolution for enhanced_mac is not necessary."
                    " Please use regular mac"
                )

            return self._acceleration

    def get_buffer_size(self, acceleration):
        """Return the mac buffer size if needed"""
        if acceleration is None:
            acceleration = self._get_acceleration()

        if acceleration  == "mac":
            return (
                1,
                self.output_size[2] * self.output_size[3],
                self.kernel_shape[2] * self.kernel_shape[3]
            )
        if acceleration  == "1x1_mac":
            return (
                1, self.output_size[2] * self.output_size[3], self.input_size[1]
            )
        if acceleration  == "1x1_mpy":
            return (
                1, self.input_size[1], self.output_size[2] * self.output_size[3]
            )

        return None

    def flip(self):
        """If the layer should flip the weights"""
        if self._get_acceleration() in ("fir", "enhanced_fir"):
            return True

        return False

    def get_calibration(self):
        """Get the list of acceleration method for calibration"""
        calibration_list = ["fir", "mac", "enhanced_fir"]

        # No need to do enhanced mac for depth-wise convolution.
        if self.group == 1:
            calibration_list.append("enhanced_mac")

        if self.kernel_shape[-1] == 1:
            calibration_list += ["1x1_mpy", "1x1_mac"]

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

        # Make sure all lea buffers are multiple of 2
        if opt_config["global_mem_buffer"]:
            if acceleration == "1x1_mac":
                lea_src_size = self.input_size[1]
                lea_src_size += (lea_src_size % 2)
                lea_flt_size = self.input_size[1]
                lea_flt_size += (lea_flt_size % 2)
                lea_tmp_size = 0 # no need for lea_tmp buffer

                lea_src_size = _size_converter(lea_src_size)
                lea_flt_size = _size_converter(lea_flt_size)
                lea_dst_size = (
                    opt_config["lea_size"] - lea_src_size - lea_flt_size
                )
                if lea_dst_size < 0:
                    raise ValueError("LEA array size noy big enough")
            elif acceleration == "1x1_mpy":
                if opt_config["lea_size"] < 3 * mul:
                    raise ValueError("LEA array size noy big enough")
                # 3 * size will always be smaller than opt_config["lea_size"]
                size = math.floor(opt_config["lea_size"] / 3) - mul
                size = _size_converter(size)
                lea_tmp_size = 0 # no need for lea_tmp buffer
                lea_src_size = size
                lea_flt_size = size
                lea_dst_size = size
            elif acceleration == "mac":
                lea_src_size = get_stride(self.kernel_shape, 1)
                lea_src_size += (lea_src_size % 2)
                lea_flt_size = get_stride(self.kernel_shape, 1)
                lea_flt_size += (lea_flt_size % 2)

                lea_src_size = _size_converter(lea_src_size)
                lea_flt_size = _size_converter(lea_flt_size)

                if self.group == 1:
                    size = math.floor(
                        (opt_config["lea_size"] - lea_src_size -
                            lea_flt_size) / 2
                    ) - mul
                    if size < 0:
                        raise ValueError("LEA array size noy big enough")
                    size = _size_converter(size)
                    lea_tmp_size = size
                    lea_dst_size = size
                else:
                    size = _size_converter(opt_config["lea_size"] -
                        lea_src_size - lea_flt_size)
                    lea_tmp_size = 0 # no need for lea_tmp buffer
                    lea_dst_size = size
            elif acceleration == "enhanced_mac":
                if opt_config["lea_size"] < 2 * mul:
                    raise ValueError("LEA array size noy big enough")
                # 2 * size will always be smaller than opt_config["lea_size"]
                size = math.floor(opt_config["lea_size"] / 2) - mul
                size = _size_converter(size)
                lea_tmp_size = 0 # no need for lea_tmp buffer
                lea_src_size = size
                lea_flt_size = size
                lea_dst_size = 0 # no need for lea_dst buffer
            elif acceleration == "enhanced_fir":
                s = self.kernel_shape[3]
                if s % 2:
                    s += 1
                lea_flt_size = _size_converter(self.kernel_shape[2] * s)
                lea_size = opt_config["lea_size"] - lea_flt_size
                if lea_size < 3 * mul:
                    raise ValueError("LEA array size noy big enough")
                # 3 * size will always be smaller than opt_config["lea_size"]
                size = math.floor(lea_size / 3) - mul
                size = _size_converter(size)
                lea_tmp_size = size
                lea_src_size = size
                lea_dst_size = size

            else: # fir
                lea_src_size = self.input_size[3] + 2
                s = self.kernel_shape[3]
                if s % 2:
                    s += 1
                lea_flt_size = self.kernel_shape[2] * s
                if self.strides[0] * self.strides[1] > 1:
                    # we need to make it bigger for deinterleave part
                    lea_tmp_size = self.input_size[3] + 2
                else:
                    lea_tmp_size = self.output_size[3] + 2

                lea_src_size = _size_converter(lea_src_size)
                lea_flt_size = _size_converter(lea_flt_size)
                lea_tmp_size = _size_converter(lea_tmp_size)
                lea_dst_size = lea_dst_size = (
                    opt_config["lea_size"] - lea_src_size - lea_flt_size -
                    lea_tmp_size
                )
        else:
            lea_src_size = opt_config["lea_size"]
            lea_flt_size = opt_config["lea_size"]
            lea_tmp_size = opt_config["lea_size"]
            lea_dst_size = opt_config["lea_size"]

        return lea_src_size, lea_flt_size, lea_tmp_size, lea_dst_size

    def get_code(self, name, jinja_dir, opt_config, qf, acceleration):
        """Get the code for the layer"""
        if acceleration is None:
            acceleration = self._get_acceleration()

        if name is None:
            name = self.name

        path = os.path.join(jinja_dir, "conv")
        path = os.path.join(path, acceleration + ".jinja")

        sizes = self._get_size(opt_config, acceleration)
        lea_src_size, lea_flt_size, lea_tmp_size, lea_dst_size = sizes

        if "intermittent" in path and acceleration == "mac":
            raise NotImplementedError(
                "Regular mac for depth-wise convolution is not completed as "
                "we need to tune the size of atomic unit that runs continuous. "
                "The process of automatic tuning is not trivial. However, we "
                "don't need to deal with this right now as using mac to do "
                "depth-wise convolution performs worse than batched-fir."
            )

        if self.has_padding:
            if self.kernel_shape[-1] == 1:
                raise NotImplementedError(
                    "Padding to input for 1x1 point-wise with mac is not "
                    "implemented as we cannot change the input which "
                    "is also used for the next layer."
                )
            padding_params = {
                "left" : self.padding[0],
                "right" : self.padding[1],
                "top" : self.padding[2],
                "bottom" : self.padding[3],
            }
        else:
            padding_params = None

        if self.input_size[1] > opt_config["lea_size"]:
            raise ValueError(
                "Input channel size has to be smaller than LEA size"
            )

        has_adaptive_gen_mem = False
        if opt_config["adaptive_gen_mem"]:
            has_adaptive_gen_mem = (has_adaptive_gen_mem or (
                padding_params is not None and (self.input_size[3] -
                    padding_params["left"] - padding_params["right"] <
                    opt_config["adaptive_gen_mem_size"])
                )
            )
            has_adaptive_gen_mem = (has_adaptive_gen_mem or
                self.kernel_shape[3] < opt_config["adaptive_gen_mem_size"]
            )
            has_adaptive_gen_mem = (has_adaptive_gen_mem or
                self.output_size[3] < opt_config["adaptive_gen_mem_size"]
            )
            has_adaptive_gen_mem = (has_adaptive_gen_mem or
                self.input_size[3] < opt_config["adaptive_gen_mem_size"]
            )

            has_adaptive_gen_mem = (has_adaptive_gen_mem or
                (get_stride(self.output_size, 1) % lea_dst_size) <
                opt_config["adaptive_gen_mem_size"]
            )

            has_adaptive_gen_mem = (has_adaptive_gen_mem or
                (((self.input_size[3] - self.kernel_shape[3]) /
                    self.strides[1]) + 1) < opt_config["adaptive_gen_mem_size"]
            )

        io_qf, weight_qf = qf

        # TODO: not sure if LEA deinterleave works with odd number of channels
        params = {
            "layer_name" : name,
            "lea_opt" : opt_config["lea_opt"],
            "dma_opt" : opt_config["dma_opt"],
            "in_ch" : self.input_size[1],
            "out_ch" : self.output_size[1],
            "flt_len" : get_stride(self.kernel_shape, 1),
            "out_len" : get_stride(self.output_size, 1),
            "in_len" : get_stride(self.input_size, 1),
            "flt_size" : self.kernel_shape[2],
            "flt_col_size" : self.kernel_shape[3],
            "in_ch_flt_len" : (
                self.input_size[1] * get_stride(self.kernel_shape, 1)),
            "in_line_size" : self.input_size[3], 
            "out_line_size" : self.output_size[3],
            "in_line_num" : self.input_size[2],
            "out_line_num" : self.output_size[2],
            "lea_src_size" : lea_src_size,
            "lea_flt_size" : lea_flt_size,
            "lea_tmp_size" : lea_tmp_size,
            "lea_dst_size" : lea_dst_size,
            "lea_buffer_size" : opt_config["lea_size"],
            "qf" : weight_qf,
            "padding" : padding_params,
            "has_adaptive_gen_mem" : has_adaptive_gen_mem,
            "global_mem_buffer" : opt_config["global_mem_buffer"],
            "stride_row" : self.strides[0],
            "stride_col" : self.strides[1],
            "group" : self.group,
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
