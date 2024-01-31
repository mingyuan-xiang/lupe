"""Supported layer types"""

from enum import Enum, auto

class LupeType(Enum):
    """Supported layer types"""
    CONV2D = auto()
    FC = auto()
    MAX_POOL = auto()
    RELU = auto()
    RESHAPE = auto()
    TANH = auto()

def get_lupe_type(onnx_type):
    """Get the Lupe supported type based on the ONNX operation type
    
    Args:
        onnx_type: The ONNX operation type
    
    Returns:
        The Lupe type
    """
    if onnx_type == 'Conv':
        return LupeType.CONV2D
    if onnx_type == 'Gemm':
        return LupeType.FC
    if onnx_type == 'MaxPool':
        return LupeType.MAX_POOL
    if onnx_type == 'Relu':
        return LupeType.RELU
    if onnx_type == 'Reshape':
        return LupeType.RESHAPE
    if onnx_type == 'Tanh':
        return LupeType.TANH

    raise ValueError(f'Unsupported ONNX operation type: {onnx_type}')
