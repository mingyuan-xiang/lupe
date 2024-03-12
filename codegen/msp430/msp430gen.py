"""The interface for the code generation for msp430"""
import os

from .maingen import maingen
from .modelgen import modelgen
from .weightgen import weightgen
from .arrgen import arrgen

class MSP430Gen:
    """The codegen class"""

    def __init__(self, code_dir, opt_config, graph, add_timer=True):
        """Initialize the codegen class"""
        self.config = self._parse_config(opt_config)
        self.code_dir = code_dir
        self.add_timer = add_timer
        self.graph = graph

    def _parse_config(self, opt_config):
        """Parse the configuration file"""
        return {}


    def gen(self, model_name, dataset_size, print_freq=100, loc="hi"):
        """Generate the code"""
        # Generate the main file
        maingen(
            self.code_dir, model_name, dataset_size,
            print_freq=print_freq, add_timer=self.add_timer
        )

        # Generate the model file
        modelgen(self.code_dir, model_name, self.graph)

        # Generate the weight and bias files
        params_dir = os.path.join(self.code_dir, "params")
        if not os.path.exists(params_dir):
            os.makedirs(params_dir)
            os.makedirs(os.path.join(params_dir, "include"))
        weightgen(params_dir, self.graph, loc=loc)

        # Generate the buffer files
        buffer_dir = os.path.join(self.code_dir, "buffer")
        if not os.path.exists(buffer_dir):
            os.makedirs(buffer_dir)
            os.makedirs(os.path.join(buffer_dir, "include"))
        arrgen(buffer_dir, self.graph, loc=loc)

        # Generate the layer code
        layer_dir = os.path.join(self.code_dir, "layers")
        if not os.path.exists(layer_dir):
            os.makedirs(layer_dir)
            os.makedirs(os.path.join(layer_dir, "include"))
