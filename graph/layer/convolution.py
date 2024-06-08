"""Convolution layer"""

import os

from jinja2 import Template

from .layer import LupeLayer
from .layer_utils import get_onnx_attr, name_conversion
from .helpers import get_stride

# TODO: Deal with group for separable convolution

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
        self.padding = tuple(pads.ints) if pads else None
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

        self._acceleration = self._decide_acceleration()

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

    def get_buffer_size(self):
        """Return the mac buffer size if needed"""
        if self._acceleration == "mac":
            return (1, self.input_size[2] * self.input_size[3],
                self.kernel_shape[3] * self.kernel_shape[3]
            )

        return None

    def flip(self):
        """If the layer should flip the weights"""
        if self._acceleration == "fir":
            return True

        return False

    def _decide_acceleration(self):
        """Decide how which operation to use"""
        # TODO: We should do something smarter for the decider
        if ("adaptive_gen_lea" not in self.opt_config or
            not self.opt_config["adaptive_gen_lea"]):
            return "fir"

        if self.kernel_shape[-1] == 5:
            if self.input_size[-1] < 14:
                return "mac"

            return "fir"


    def get_code(self, jinja_dir, opt_config, qf):
        """Get the code for the layer"""
        path = os.path.join(jinja_dir, "conv.jinja")

        if self.has_padding:
            padding_params = {
                "left" : self.padding[0],
                "right" : self.padding[1],
                "top" : self.padding[2],
                "bottom" : self.padding[3],
            }
        else:
            padding_params = None

        has_adaptive_gen_mem = False
        if opt_config["adaptive_gen_mem"]:
            has_adaptive_gen_mem = (has_adaptive_gen_mem or (
                padding_params is not None and (self.input_size[3] -
                    padding_params["left"] - padding_params["right"] <
                    opt_config["adaptive_gen_mem_size"])
                )
            )
            has_adaptive_gen_mem = (has_adaptive_gen_mem or
                self.kernel_shape[2] < opt_config["adaptive_gen_mem_size"]
            )
            has_adaptive_gen_mem = (has_adaptive_gen_mem or
                self.output_size[3] < opt_config["adaptive_gen_mem_size"]
            )
            has_adaptive_gen_mem = (has_adaptive_gen_mem or
                self.input_size[3] < opt_config["adaptive_gen_mem_size"]
            )

        params = {
            "layer_name" : self.name,
            "lea_opt" : opt_config["lea_opt"],
            "in_ch" : self.input_size[1],
            "out_ch" : self.output_size[1],
            "flt_len" : get_stride(self.kernel_shape, 1),
            "out_len" : get_stride(self.output_size, 1),
            "in_len" : get_stride(self.input_size, 1),
            "flt_size" : self.kernel_shape[2],
            "in_line_size" : self.input_size[3], 
            "out_line_size" : self.output_size[3],
            "in_line_num" : self.input_size[2],
            "out_line_num" : self.output_size[2],
            "qf" : qf,
            "padding" : padding_params,
            "lea_min_size" : min(
                opt_config["lea_src_size"], opt_config["lea_dst_size"],
                opt_config["lea_flt_size"]),
            "has_adaptive_gen_mem" : has_adaptive_gen_mem,
        }

        if opt_config["adaptive_gen_mem"]:
            params["adaptive_gen_mem_size"] = (
                opt_config["adaptive_gen_mem_size"]
            )

        # select correct operators
        if opt_config["adaptive_gen_lea"]:
            params["acceleration"]  = self._acceleration
        else:
            params["acceleration"] = "fir"

        with open(path, "r", encoding="utf-8") as file:
            template = file.read()
            j_template = Template(template)
            code_str = j_template.render(params)

            return code_str
