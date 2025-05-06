"""Pooling layer"""

from abc import abstractmethod
import os

from jinja2 import Template

from .layer import LupeLayer
from .layer_utils import get_onnx_attr, name_conversion
from .helpers import get_stride

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

    def _get_name(self, node):
        """Get the name of the layer"""
        return name_conversion(node.name)

    def has_weights(self):
        """If the layer has weights"""
        return False

    def get_buffer_size(self, acceleration):
        """If the layer needs extra buffer. Return the buffer shape tuple"""
        return None

    @abstractmethod
    def _get_template_name(self):
        """Get the jinja template name for the pooling layer"""

    def get_code(self, name, jinja_dir, opt_config, qf, acceleration):
        """Get the code for the layer"""
        if name is None:
            name = self.name

        path = os.path.join(jinja_dir, self._get_template_name())

        params = {
            "layer_name" : name,
            "height" : self.kernel_shape[0],
            "width" : self.kernel_shape[1],
            "in_ch" : self.input_size[1],
            "rows" : self.input_size[2],
            "cols" : self.input_size[3],
            "in_ch_stride" : get_stride(self.input_size, 1),
            "out_ch_stride" : get_stride(self.output_size, 1),
            "in_row_stride" : get_stride(self.input_size, 2),
            "out_row_stride" : get_stride(self.output_size, 2),
        }

        with open(path, "r", encoding="utf-8") as file:
            template = file.read()
            j_template = Template(template)
            code_str = j_template.render(params)

            return code_str

class AvgPooling(Pooling):
    """Average pooling layer"""
    def _get_template_name(self):
        """Return how to update the pooling layer"""
        return "avg_pooling.jinja"

class GlobalAvgPooling(AvgPooling):
    """Global Average pooling layer"""
    def _register(self, node):
        """Set kernel sizes for the Global Average pooling layer"""
        self.kernel_shape = (self.input_size[2], self.input_size[3])

class MaxPooling(Pooling):
    """Max pooling layer"""
    def _get_template_name(self):
        """Return how to update the pooling layer"""
        return "max_pooling.jinja"
