"""Pooling layer"""

from .layer import LupeLayer
from .layer_utils import get_onnx_attr

class Pooling(LupeLayer):
    """Pooling layer"""
    def _register(self, node):
        """Register the layer"""
        # kernel_shape
        kernel_shape = get_onnx_attr(node, "kernel_shape")
        self.kernel_shape = tuple(kernel_shape.ints) if kernel_shape else None
        # pads
        pads = get_onnx_attr(node, "pads")
        self.pads = tuple(pads.ints) if pads else None
        # strides
        strides = get_onnx_attr(node, "strides")
        self.strides = tuple(strides.ints) if strides else None

    def __str__(self):
        s = f"{self.name}: {self.__class__.__name__}("
        s += f"kernel_shape={self.kernel_shape}, "
        s += f"pads={self.pads}, "
        s += f"strides={self.strides}"
        s += ")"

        return s

class AvgPooling(Pooling):
    """Average pooling layer"""


class MaxPooling(Pooling):
    """Max pooling layer"""
