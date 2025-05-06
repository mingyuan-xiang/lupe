"""Clip layer"""

import os

from jinja2 import Template


from .layer import LupeLayer
from .layer_utils import name_conversion

class Clip(LupeLayer):
    """Clip layer"""
    def _get_name(self, node):
        """name of the clip layer"""
        return name_conversion(node.name)

    def has_weights(self):
        """If the layer has weights"""
        return False

    def get_buffer_size(self, acceleration):
        """If the layer needs extra buffer. Return the buffer shape tuple"""
        return None

    def get_code(self, name, jinja_dir, opt_config, qf, acceleration):
        """Get the code for the layer"""
        if name is None:
            name = self.name

        if not (isinstance(self.min, (int, float)) or
            isinstance(self.max, (int, float))):
            raise TypeError

        path = os.path.join(jinja_dir, "clip.jinja")

        params = {
            "layer_name" : name,
            "min" : max(int(self.min * (2 ** (15 - qf))), -2**15+1),
            "max" : min(int(self.max * (2 ** (15 - qf))), 2**15-2),
        }

        with open(path, "r", encoding="utf-8") as file:
            template = file.read()
            j_template = Template(template)
            code_str = j_template.render(params)

            return code_str

    def __str__(self):
        s = f"{self.name}: Clip(min={self.min}, max={self.max})"

        return s
