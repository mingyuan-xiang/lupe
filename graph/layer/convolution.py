"""Convolution layer"""

from layer import LupeLayer, get_onnx_attr

class Convolution(LupeLayer):
    """Convolution layer"""
    def _register(self, node):
        """Register the layer"""
        # dilations
        dilations = get_onnx_attr(node, "dilations")
        self.dilations = list(dilations.ints) if dilations else None
        # group
        group = get_onnx_attr(node, "group")
        self.group = group.i if group else None
        # kernel_shape
        kernel_shape = get_onnx_attr(node, "kernel_shape")
        self.kernel_shape = list(kernel_shape.ints) if kernel_shape else None
        # pads
        pads = get_onnx_attr(node, "pads")
        self.pads = list(pads.ints) if pads else None
        # strides
        strides = get_onnx_attr(node, "strides")
        self.strides = list(strides.ints) if strides else None

