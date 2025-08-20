# SPDX-License-Identifier: AGPL-3.0-or-later
# Copyright (C) 2025 Mingyuan Xiang

from abc import abstractmethod
import os

from jinja2 import Template

from .layer import LupeLayer
from .layer_utils import name_conversion
from .helpers import get_stride

class Activation(LupeLayer):
    """Activation layer"""

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
    def _get_update(self, in_var):
        """Return how to update the activation layer
        
        Args:
            in_var (str): The input variable name
        """

    def get_code(self, name, jinja_dir, opt_config, qf, acceleration):
        """Get the code for the layer"""
        if name is None:
            name = self.name

        path = os.path.join(jinja_dir, "activation.jinja")

        params = {
            "layer_name" : name,
            "in_stride" : get_stride(self.input_size, 0),
            "update_code" : self._get_update( "val"),
        }

        with open(path, "r", encoding="utf-8") as file:
            template = file.read()
            j_template = Template(template)
            code_str = j_template.render(params)

            return code_str

class Relu(Activation):
    """Relu layer"""
    def _get_update(self, in_var):
        return f"{in_var} < 0 ? 0 : {in_var}"

class Tanh(Activation):
    """Tanh layer"""
    def _get_update(self, in_var):
        raise NotImplementedError
