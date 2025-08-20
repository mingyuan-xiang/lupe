# SPDX-License-Identifier: AGPL-3.0-or-later
# Copyright (C) 2025 Mingyuan Xiang

import onnx.numpy_helper

from .layer import LupeLayer
from .layer_utils import name_conversion
from .helpers import get_qf

class Weight(LupeLayer):
    """Tensor layer"""
    def _register(self, node):
        """Register the layer"""
        self.shape = list(node.dims)
        self.data = onnx.numpy_helper.to_array(node)
        self.qf = get_qf(self.data, 2.5, qf_offset=self.qf_offset)

    def _get_name(self, node):
        """For weights and biases, the name is the name of the tensor"""
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
