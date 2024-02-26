"""Weight and bias"""

import onnx.numpy_helper

from .layer import LupeLayer
from .layer_utils import name_conversion

class tensor(LupeLayer):
    """Tensor layer"""
    def _register(self, node):
        """Register the layer"""
        self.shape = list(node.dims)
        self.data = onnx.numpy_helper.to_array(node)

    def _get_name(self, node):
        """For weights and biases, the name is the name of the tensor"""
        return name_conversion(node.name)


class Weight(tensor):
    """Weight"""

class Bias(tensor):
    """Bias"""
