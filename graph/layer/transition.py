"""Transition layers"""

from abc import abstractmethod
import os

from jinja2 import Template

from .layer import LupeLayer
from .layer_utils import name_conversion

class Transition(LupeLayer):
    """Transition layer"""

    def _get_name(self, node):
        """Get the name of the layer"""
        return name_conversion(node.name)

    def has_weights(self):
        """If the layer has weights"""
        return False

    def __str__(self):
        return f"{self.name}: {self.__class__.__name__}()"

    @abstractmethod
    def _get_update(self):
        """Get the update for the layer"""

    def get_code(self, jinja_dir, opt_config):
        """Get the code for the layer"""
        path = os.path.join(jinja_dir, "transition.jinja")

        params = {
            "layer_name" : self.name,
            "update" : self._get_update(),
        }

        with open(path, "r", encoding="utf-8") as file:
            template = file.read()
            j_template = Template(template)
            code_str = j_template.render(params)

            return code_str

class Flatten(Transition):
    """Flatten layer"""
    def _get_update(self):
        return "/* The data is stored in 1D array. */"
