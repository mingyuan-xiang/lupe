"""Pooling layer"""

from layer import LupeLayer, get_onnx_attr

class Pooling(LupeLayer):
    """Pooling layer"""
    def _register(self, node):
        """Register the layer"""
        # kernel_shape
        kernel_shape = get_onnx_attr(node, "kernel_shape")
        self.kernel_shape = list(kernel_shape.ints) if kernel_shape else None
        # pads
        pads = get_onnx_attr(node, "pads")
        self.pads = list(pads.ints) if pads else None
        # strides
        strides = get_onnx_attr(node, "strides")
        self.strides = list(strides.ints) if strides else None

class AvgPooling(Pooling):
    """Average pooling layer"""


class MaxPooling(Pooling):
    """Max pooling layer"""
