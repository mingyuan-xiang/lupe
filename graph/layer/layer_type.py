"""Supported layer types"""

from enum import Enum, auto

class LupeType(Enum):
    """Supported layer types"""
    AVG_POOL = auto()
    BIAS = auto()
    CONSTANT = auto()
    CONV2D = auto()
    FC = auto()
    FLATTEN = auto()
    INPUT = auto()
    MAX_POOL = auto()
    OUTPUT = auto()
    RELU = auto()
    RESHAPE = auto()
    TANH = auto()
    WEIGHT = auto()

def get_lupe_type(onnx_type):
    """Get the Lupe supported type based on the ONNX operation type
    
    Args:
        onnx_type: The ONNX operation type
    
    Returns:
        The Lupe type
    """
    if onnx_type == "AveragePool":
        ty = LupeType.AVG_POOL
    elif onnx_type == "Conv":
        ty = LupeType.CONV2D
    elif onnx_type == "Gemm":
        ty = LupeType.FC
    elif onnx_type == "MaxPool":
        ty = LupeType.MAX_POOL
    elif onnx_type == "Relu":
        ty = LupeType.RELU
    elif onnx_type == "Reshape":
        ty = LupeType.RESHAPE
    elif onnx_type == "Tanh":
        ty = LupeType.TANH
    elif onnx_type == "Constant":
        ty = LupeType.CONSTANT
    elif onnx_type == "Flatten":
        ty = LupeType.FLATTEN
    else:
        raise ValueError(f"Unsupported ONNX operation type: {onnx_type}")

    return ty

def lupe_type_to_string(lupe_type):
    """Convert the LupeType to a string
    
    Args:
        lupe_type: The LupeType

    Returns:
        The string representation of the LupeType
    """
    string = ""

    if lupe_type == LupeType.BIAS:
        string = "Bias"
    elif lupe_type == LupeType.CONSTANT:
        string = "Constant"
    elif lupe_type == LupeType.CONV2D:
        string = "Conv2D"
    elif lupe_type == LupeType.FC:
        string = "Fully Connected"
    elif lupe_type == LupeType.INPUT:
        string = "Input"
    elif lupe_type == LupeType.MAX_POOL:
        string = "Max Pool"
    elif lupe_type == LupeType.OUTPUT:
        string = "Output"
    elif lupe_type == LupeType.RELU:
        string = "ReLU"
    elif lupe_type == LupeType.RESHAPE:
        string = "Reshape"
    elif lupe_type == LupeType.TANH:
        string = "Tanh"
    elif lupe_type == LupeType.WEIGHT:
        string = "Weight"
    else:
        raise ValueError(f"Unsupported LupeType: {lupe_type}")

    return string
