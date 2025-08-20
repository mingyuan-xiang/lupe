# SPDX-License-Identifier: AGPL-3.0-or-later
# Copyright (C) 2025 Mingyuan Xiang

import os
import math

from jinja2 import Template

from .layer import LupeLayer
from .layer_utils import name_conversion
from .helpers import get_stride

class Add(LupeLayer):
    """Add layer"""
    def _register(self, node):
        self.input = [
            name_conversion(node.input[0])[:-len("_output_0")],
            name_conversion(node.input[1])[:-len("_output_0")],
        ]


    def _get_name(self, node):
        """name of the add layer"""
        return name_conversion(node.name)

    def has_weights(self):
        """If the layer has weights"""
        return False

    def get_buffer_size(self, acceleration):
        """If the layer needs extra buffer. Return the buffer shape tuple"""
        return None

    def get_code(self, name, jinja_dir, opt_config, qf, acceleration):
        """Get the code for the layer"""
        if name is None:
            name = self.name

        path = os.path.join(jinja_dir, "add.jinja")

        if opt_config["global_mem_buffer"]:
            lea_src_size = math.floor((opt_config["lea_size"] - 2) / 2)
        else:
            lea_src_size = opt_config["lea_size"]

        # Make sure all lea buffers are multiple of 2
        lea_src_size += (lea_src_size % 2)

        params = {
            "layer_name" : name,
            "lea_opt" : opt_config["lea_opt"],
            "lea_src_size" : lea_src_size,
            "input_size" : get_stride(self.input_size, 0),
            "global_mem_buffer" : opt_config["global_mem_buffer"],
        }

        with open(path, "r", encoding="utf-8") as file:
            template = file.read()
            j_template = Template(template)
            code_str = j_template.render(params)

            return code_str

    def __str__(self):
        s = f"{self.name}: Add(input0={self.input[0]}, input1={self.input[1]})"

        return s
