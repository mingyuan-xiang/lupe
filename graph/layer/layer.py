"""The layer node in the graph"""

from abc import ABC, abstractmethod

from .layer_type import LupeType, get_lupe_type
from .convolution import Convolution
from .in_out import InOut
from .pooling import AvgPooling, MaxPooling
from .activation import Relu, Tanh
from .transition import Flatten
from .fully_connected import FullyConnected

class LupeLayer(ABC):
    def __init__(self, name, node):
        """Initialize the layer
        
        Args:
            name: The name of the layer
            node: The ONNX node
        """
        self.name = name
        self._register(node)

    @abstractmethod
    def _register(self, node):
        """Register the layer"""

    @abstractmethod
    def get_code(self):
        """Get the code for the layer"""

def get_layer(node, node_list, graph):
    """Get the layer from the node
    
    Args:
        node: The ONNX node
        graph: complete graph as a dictionary
        node_list: the dictionary of nodes

    Returns:
        The dictionary of layers
    """

    lupe_type = get_lupe_type(node.op_type)
    inputs = node.input
    node_name = node.output[0]

    graph[inputs[0]].append(node_name)

    # Add output node
    graph[node_name] = []
    node_list[node_name] = _get_layer_constructor(lupe_type)(
        node_name, lupe_type, node
    )

    return node_list, graph

def _get_layer_constructor(lupe_type):
    """Get the layer constructor"""
    if lupe_type == LupeType.CONV2D:
        layer = Convolution
    elif lupe_type in (LupeType.INPUT, LupeType.OUTPUT):
        layer = InOut
    elif lupe_type == LupeType.AVG_POOL:
        layer = AvgPooling
    elif lupe_type == LupeType.MAX_POOL:
        layer = MaxPooling
    elif lupe_type == LupeType.RELU:
        layer = Relu
    elif lupe_type == LupeType.TANH:
        layer = Tanh
    elif lupe_type == LupeType.FLATTEN:
        layer = Flatten
    elif lupe_type == LupeType.FC:
        layer = FullyConnected
    else:
        raise ValueError(f"Unsupported Lupe type: {lupe_type}")

    return layer

def get_onnx_attr(node, s):
    """Get the ONNX attribute
    
    Args:
        node: The ONNX node
        s: The attribute name
    """
    return next(
        (attr for attr in node.attribute if attr.name == s), None
    )