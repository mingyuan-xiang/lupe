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

    def get_buffer_size(self, acceleration):
        """If the layer needs extra buffer. Return the buffer shape tuple"""
        return None

    def __str__(self):
        return f"{self.name}: {self.__class__.__name__}()"

    @abstractmethod
    def _get_update(self):
        """Get the update for the layer"""

    def get_code(self, name, jinja_dir, opt_config, qf, acceleration):
        """Get the code for the layer"""
        if name is None:
            name = self.name

        path = os.path.join(jinja_dir, "transition.jinja")

        params = {
            "layer_name" : name,
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
        code = (
            "  /* The data is stored in 1D array. So, we don't need to " +
            "do anything. */ \n"
        )

        return code
