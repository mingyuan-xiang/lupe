"""The layer node in the graph"""

from abc import ABC, abstractmethod

class LupeLayer(ABC):
    """
    The basic layer node in the graph
    """
    def __init__(self, name, node):
        """Initialize the layer
        
        Args:
            name: The name of the layer
            node: The ONNX node
        """
        self.name = name
        self._register(node)

    def _register(self, node):
        """Register the layer"""

    # @abstractmethod
    # def get_code(self):
    #     """Get the code for the layer"""

    def print(self):
        """Print the layer"""
        print(str(self))
