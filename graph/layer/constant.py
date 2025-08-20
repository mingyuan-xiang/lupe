# SPDX-License-Identifier: AGPL-3.0-or-later
# Copyright (C) 2025 Mingyuan Xiang

from .layer import LupeLayer
from .layer_utils import get_onnx_attr, name_conversion
import onnx.numpy_helper

class Constant(LupeLayer):
    """Constant layer"""
    def _register(self, node):
        val = get_onnx_attr(node, "value")
        self.value = onnx.numpy_helper.to_array(val.t).item()

    def _get_name(self, node):
        """Get the name of the layer"""
        return name_conversion(str(node.output[0]))

    def has_weights(self):
        """If the layer has weights"""
        return False

    def get_buffer_size(self, acceleration):
        """If the layer needs extra buffer. Return the buffer shape tuple"""
        return None

    def __str__(self):
        return f"{self.value}"

    def get_code(self, name, jinja_dir, opt_config, qf, acceleration):
        """Get the code for the layer"""
        raise NotImplementedError("We don't need to implement constant layer.")
