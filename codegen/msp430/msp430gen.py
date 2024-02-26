"""The interface for the code generation for msp430"""

from .maingen import maingen
from .modelgen import modelgen

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


    def gen(self, model_name, dataset_size, print_freq=100):
        """Generate the code"""
        maingen(
            self.code_dir, model_name, dataset_size,
            print_freq=print_freq, add_timer=self.add_timer
        )
        modelgen(self.code_dir, model_name, self.graph)
