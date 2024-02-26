"""Activation layers"""

from .layer import LupeLayer
from .layer_utils import name_conversion

class Activation(LupeLayer):
    """Activation layer"""

    def _get_name(self, node):
        """Get the name of the layer"""
        return name_conversion(node.name)

class Relu(Activation):
    """Relu layer"""

class Tanh(Activation):
    """Tanh layer"""
