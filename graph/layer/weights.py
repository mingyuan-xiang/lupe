"""Weight and bias"""

import onnx.numpy_helper

from .layer import LupeLayer

class tensor(LupeLayer):
    """Tensor layer"""
    def _register(self, node):
        """Register the layer"""
        self.shape = list(node.dims)
        self.data = onnx.numpy_helper.to_array(node)


class Weight(tensor):
    """Weight"""

class Bias(tensor):
    """Bias"""
