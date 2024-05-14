"""The interface for the code generation for msp430"""

import os

from .maingen import maingen
from .modelgen import modelgen
from .weightgen import weightgen
from .arrgen import arrgen
from .utilsgen import utilsgen
from .layergen import layergen
from .makefilegen import makefilegen

import pprint

class MSP430Gen:
    """The codegen class"""
    def __init__(self, code_dir, opt_config, graph, qf, add_timer=True):
        """Initialize the codegen class"""
        self.opt_config = self._parse_config(opt_config)
        self.code_dir = code_dir
        # Create the source directory
        self.src_dir = os.path.join(code_dir, "src")
        if not os.path.exists(self.src_dir):
            os.makedirs(self.src_dir)
        self.add_timer = add_timer
        self.graph = graph
        self.debug = False
        self.debug_input = None
        self.debug_input_label = None
        self.qf = qf

    def _parse_config(self, opt_config):
        """Parse the configuration file. Set default to False if not present."""
        if "dma_opt" not in opt_config:
            opt_config["dma_opt"] = False
        if "lea_flt_size" not in opt_config:
            opt_config["lea_flt_size"] = 100
        if "lea_src_size" not in opt_config:
            opt_config["lea_src_size"] = 100
        if "lea_dst_size" not in opt_config:
            opt_config["lea_dst_size"] = 100
        if "prop_const" not in opt_config:
            opt_config["prop_const"] = False
        if "saturation" not in opt_config:
            opt_config["saturation"] = "none"
        if not opt_config["saturation"] in ("none", "scaled", "32bit"):
            raise NotImplementedError("Saturation model of " +
                f"`{opt_config['saturation']}` is not supported."
            )

        return opt_config

    def gen(self, model_name, dataset_size, print_freq=100, loc="hi"):
        """Generate the code"""
        # Create the include directory
        if not os.path.exists(os.path.join(self.src_dir, "include")):
            os.makedirs(os.path.join(self.src_dir, "include"))

        # Generate the main file
        maingen(
            self.src_dir, model_name, dataset_size,
            print_freq=print_freq, add_timer=self.add_timer,
            debug=self.debug, label=self.debug_input_label
        )

        # Generate the model file
        modelgen(self.src_dir, self.graph, self.debug)

        # Generate the weight and bias files
        params_dir = os.path.join(self.src_dir, "params")
        if not os.path.exists(params_dir):
            os.makedirs(params_dir)
            os.makedirs(os.path.join(params_dir, "include"))
        flt_sizes = weightgen(params_dir, self.graph, self.qf, loc=loc)

        # Generate the buffer files
        buffer_dir = os.path.join(self.src_dir, "buffer")
        if not os.path.exists(buffer_dir):
            os.makedirs(buffer_dir)
            os.makedirs(os.path.join(buffer_dir, "include"))
        arrgen(buffer_dir, self.graph, loc=loc, debug_input=self.debug_input)

        # Generate the layer code
        layer_dir = os.path.join(self.src_dir, "layers")
        if not os.path.exists(layer_dir):
            os.makedirs(layer_dir)
            os.makedirs(os.path.join(layer_dir, "include"))
        utilsgen(layer_dir, self.opt_config, flt_sizes)
        layergen(layer_dir, self.graph, self.opt_config, self.qf, self.debug)

        # Generate the makefile
        makefilegen(self.code_dir, self.graph)

    def print_config(self):
        """Print the configuration"""
        pprint.pprint(dict(self.opt_config.items()))

    def setup_debug_info(self, input_arr, label):
        """Set up input array and label"""
        self.debug = True
        self.debug_input = input_arr
        self.debug_input_label = label
