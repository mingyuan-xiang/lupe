"""Clip layer"""

import os

from jinja2 import Template


from .layer import LupeLayer
from .layer_utils import name_conversion

class Clip(LupeLayer):
    """Tensor layer"""
    def _get_name(self, node):
        """For weights and biases, the name is the name of the tensor"""
        return name_conversion(node.name)

    def has_weights(self):
        """If the layer has weights"""
        return False

    def get_buffer_size(self):
        """If the layer needs extra buffer. Return the buffer shape tuple"""
        return None

    def get_code(self, jinja_dir, opt_config, qf):
        """Get the code for the layer"""
        if not (isinstance(self.min, (int, float)) or
            isinstance(self.max, (int, float))):
            raise TypeError

        path = os.path.join(jinja_dir, "clip.jinja")

        params = {
            "layer_name" : self.name,
            "min" : int(self.min * (2 ** (15 - qf))),
            "max" : int(self.max * (2 ** (15 - qf))),
        }

        with open(path, "r", encoding="utf-8") as file:
            template = file.read()
            j_template = Template(template)
            code_str = j_template.render(params)

            return code_str

    def __str__(self):
        s = f"{self.name}: Clip(min={self.min}, max={self.min})"

        return s
