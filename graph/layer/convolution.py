"""Convolution layer"""

from .layer import LupeLayer
from .layer_utils import get_onnx_attr, name_conversion

# TODO: Deal with group for separable convolution

class Convolution2D(LupeLayer):
    """2D Convolution layer"""
    def _register(self, node):
        """Register the layer"""
        # group
        group = get_onnx_attr(node, "group")
        self.group = group.i if group else None
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
        s = f"{self.name}: Convolution2D("
        s += f"kernel_shape={self.kernel_shape}, "
        s += f"pads={self.pads}, "
        s += f"strides={self.strides}"
        if self.group:
            s += f", group={self.group}"
        s += ")"

        return s

    def _get_name(self, node):
        """Get the name of the layer"""
        for i in node.input:
            if "weight" in i:
                return name_conversion(i)[:-len("_weight")]

        raise NameError(
            f"The convolution layer {node.name} doesn't have weights"
        )

    def has_weights(self):
        """If the layer has weights"""
        return True
