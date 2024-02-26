"""The layer node in the graph"""

from abc import ABC, abstractmethod

class LupeLayer(ABC):
    """
    The basic layer node in the graph
    """
    def __init__(self, node):
        """Initialize the layer
        
        Args:
            name: The name of the layer
            node: The ONNX node
        """
        self.name = self._get_name(node)
        self._register(node)

    @abstractmethod
    def _get_name(self, node):
        """Get the name of the layer"""

    def _register(self, node):
        """Register the layer"""

    def print(self):
        """Print the layer"""
        print(str(self))
