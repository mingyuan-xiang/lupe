"""The layer node in the graph"""

from .layer_type import LupeType, get_lupe_type

from dataclasses import dataclass

@dataclass
class LupeLayer:
    """The layer node in the graph"""
    name : str
    type : LupeType

def get_layer(node, nodeList, graph):
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

    # Check if the node name is a number. If so, then it is an output node
    try:
        float(node_name)
        node_name = "output"
    except ValueError:
        pass

    if lupe_type in (LupeType.CONV2D, LupeType.FC):
        if inputs[0] not in nodeList:
            # Input tensor
            nodeList[inputs[0]] = LupeLayer(inputs[0], LupeType.INPUT)
            graph[inputs[0]] = []
        graph[inputs[0]].append(node_name)

        # Add weight and bias nodes
        nodeList[inputs[1]] = LupeLayer(inputs[1], LupeType.WEIGHT)
        graph[inputs[1]] = [node_name]
        nodeList[inputs[2]] = LupeLayer(inputs[2], LupeType.BIAS)
        graph[inputs[2]] = [node_name]

        # Add output node
        graph[node_name] = []
        nodeList[node_name] = LupeLayer(node_name, lupe_type)
    elif lupe_type in (LupeType.MAX_POOL, LupeType.TANH, LupeType.RELU):
        if inputs[0] not in nodeList:
            # Input tensor
            nodeList[inputs[0]] = LupeLayer(inputs[0], LupeType.INPUT)
            graph[inputs[0]] = []
        graph[inputs[0]].append(node_name)

        # Add output node
        graph[node_name] = []
        nodeList[node_name] = LupeLayer(node_name, lupe_type)
    elif lupe_type == LupeType.RESHAPE:
        if inputs[0] not in nodeList:
            # Input tensor
            nodeList[inputs[0]] = LupeLayer(inputs[0], LupeType.INPUT)
            graph[inputs[0]] = []
        graph[inputs[0]].append(node_name)

        # constant tensor
        nodeList[inputs[1]] = LupeLayer(inputs[1], LupeType.CONSTANT)
        graph[inputs[1]] = [node_name]

        # Add output node
        graph[node_name] = []
        nodeList[node_name] = LupeLayer(node_name, lupe_type)
    elif lupe_type == LupeType.CONSTANT:
        # Add output node
        graph[node_name] = []
        nodeList[node_name] = LupeLayer(node_name, lupe_type)
    else:
        raise ValueError(f'Unsupported Lupe type: {lupe_type}')

    return nodeList, graph
