# SPDX-License-Identifier: AGPL-3.0-or-later
# Copyright (C) 2025 Mingyuan Xiang

from .layer import LupeLayer
from .layer_utils import name_conversion

class InOut(LupeLayer):
    """Input and output layer"""

    def _register(self, node):
        """Register the layer
        
        Args:
            node: ONNX node
        """
        shape = [dim.dim_value for dim in node.type.tensor_type.shape.dim]
        # For input batch size of 1, the first dimension will be 0.
        # So, we need to change it to 1 so that it will be meaningful
        if shape[0] == 0:
            shape[0] = 1
        self.shape = tuple(shape)

    def __str__(self):
        return f"{self.name}: InOut(shape={self.shape})"

    def _get_name(self, node):
        """Get the name of the layer"""
        return name_conversion(node.name)

    def has_weights(self):
        """If the layer has weights"""
        return False

    def get_buffer_size(self, acceleration):
        """If the layer needs extra buffer. Return the buffer shape tuple"""
        return None

    def get_code(self, name, jinja_dir, opt_config, qf, acceleration):
        """This will never get called"""
        raise NotImplementedError
