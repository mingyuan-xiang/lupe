"""The layer node in the graph"""

import sys

from abc import ABC, abstractmethod
from .layer_utils import name_conversion

class LupeLayer(ABC):
    """
    The basic layer node in the graph
    """
    def __init__(self, node, node_list):
        """Initialize the layer
        
        Args:
            name: The name of the layer
            node: The ONNX node
        """
        self.name = self._get_name(node)
        self._register(node)
        self._register_weights(node, node_list)

    @abstractmethod
    def _get_name(self, node):
        """Get the name of the layer"""

    @abstractmethod
    def has_weights(self):
        """If the layer has weights"""

    def _register(self, node):
        """Register the layer"""

    def _register_weights(self, node, node_list):
        """Register the weights"""
        if self.has_weights():
            if node_list is None:
                sys.exit("node_list is None")
            for input_name in node.input:
                if "weight" in input_name:
                    weight_name = name_conversion(input_name)
                    self.weight = node_list[weight_name]
                if "bias" in input_name:
                    bias_name = name_conversion(input_name)
                    self.bias = node_list[bias_name]

    def print(self):
        """Print the layer"""
        print(str(self))
