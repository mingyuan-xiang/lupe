"""Transition layers"""

from .layer import LupeLayer
from .layer_utils import name_conversion

class Transition(LupeLayer):
    """Transition layer"""

    def _get_name(self, node):
        """Get the name of the layer"""
        return name_conversion(node.name)

class Flatten(Transition):
    """Flatten layer"""
