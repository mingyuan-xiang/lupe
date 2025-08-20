# SPDX-License-Identifier: AGPL-3.0-or-later
# Copyright (C) 2025 Mingyuan Xiang

from enum import Enum, auto
from .convolution import Convolution2D
from .in_out import InOut
from .pooling import AvgPooling, GlobalAvgPooling, MaxPooling
from .activation import Relu, Tanh
from .transition import Flatten
from .fully_connected import FullyConnected
from .weights import Weight
from .constant import Constant
from .clip import Clip
from .add import Add

class LupeType(Enum):
    """Supported layer types"""
    AVG_POOL = auto()
    CONSTANT = auto()
    CONV2D = auto()
    FC = auto()
    FLATTEN = auto()
    Global_Avg_Pool = auto()
    INPUT = auto()
    MAX_POOL = auto()
    OUTPUT = auto()
    RELU = auto()
    RESHAPE = auto()
    TANH = auto()
    WEIGHT = auto()
    CLIP = auto()
    ADD = auto()

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
    elif onnx_type == "Clip":
        ty = LupeType.CLIP
    elif onnx_type == "Add":
        ty = LupeType.ADD
    elif onnx_type == "GlobalAveragePool":
        ty = LupeType.Global_Avg_Pool
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

    if lupe_type == LupeType.AVG_POOL:
        string = "Average Pool"
    elif lupe_type == LupeType.CONSTANT:
        string = "Constant"
    elif lupe_type == LupeType.CONV2D:
        string = "Conv2D"
    elif lupe_type == LupeType.FC:
        string = "Fully Connected"
    elif lupe_type == LupeType.FLATTEN:
        string = "Flatten"
    elif lupe_type == LupeType.Global_Avg_Pool:
        string = "Global Average Pool"
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
    elif lupe_type == LupeType.CLIP:
        string = "Clip"
    elif lupe_type == LupeType.ADD:
        string = "Add"
    else:
        raise ValueError(f"Unsupported LupeType: {lupe_type}")

    return string

def get_layer_constructor(lupe_type):
    """Get the layer constructor"""
    if lupe_type == LupeType.CONV2D:
        layer = Convolution2D
    elif lupe_type in (LupeType.INPUT, LupeType.OUTPUT):
        layer = InOut
    elif lupe_type == LupeType.AVG_POOL:
        layer = AvgPooling
    elif lupe_type == LupeType.MAX_POOL:
        layer = MaxPooling
    elif lupe_type == LupeType.Global_Avg_Pool:
        layer = GlobalAvgPooling
    elif lupe_type == LupeType.RELU:
        layer = Relu
    elif lupe_type == LupeType.TANH:
        layer = Tanh
    elif lupe_type == LupeType.FLATTEN:
        layer = Flatten
    elif lupe_type == LupeType.FC:
        layer = FullyConnected
    elif lupe_type == LupeType.WEIGHT:
        layer = Weight
    elif lupe_type == LupeType.CONSTANT:
        layer = Constant
    elif lupe_type == LupeType.CLIP:
        layer = Clip
    elif lupe_type == LupeType.ADD:
        layer = Add
    else:
        raise ValueError(f"Unsupported Lupe type: {lupe_type}")

    return layer
