"""The layer node in the graph"""

from layer_type import LupeType as L

from abc import ABC, abstractmethod
from collections import deque

class LupeLayer(ABC):
    """The layer node in the graph"""
    def __init__(self, name, lupe_type):
        """Initialize the layer node in the graph
        
        Args:
            name: The name of the layer
            type: The type of the layer
            input_shape: The input of the layer
            output_shape: The output of the layer
            children: The children of the layer
        """
        self.name = name
        self.type = lupe_type
        self.children = []

    def register(self, parent):
        """Use BFS to register all the nodes in the graph
        
        Assume that the graph is acyclic for now.
        """
        for n in parent.output:
            child = LupeLayer(n.name, L.get_lupe_type(n.op_type))
            child.register(n)
            self.children.append(child)

    @abstractmethod
    def fuse(self, node1, node2):
        """Generate the layer based on the two input layers (layer fusion)"""


class Root(LupeLayer):
    """The root node in the graph"""
    def __init__(self, name, lupe_type, node_dict, input_shape):
        """Initialize the root node in the graph
        
        Args:
            name: The name of the layer
            type: The type of the layer
            input_shape: The input of the layer
            output_shape: The output of the layer
        """
        super().__init__(name, lupe_type, node_dict, input_shape)