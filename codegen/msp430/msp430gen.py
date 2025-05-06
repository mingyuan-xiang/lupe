"""The interface for the code generation for msp430"""

import os

from . import JINJA_DIR

from .maingen import maingen
from .modelgen import modelgen
from .weightgen import weightgen
from .arrgen import arrgen
from .utilsgen import utilsgen
from .layergen import layergen
from .makefilegen import makefilegen
from .intermittentgen import intermittentgen

import pprint

class MSP430Gen:
    """The codegen class"""
    def __init__(
            self, code_dir, opt_config, graph, qf, add_timer=True,
            calibration=False):
        """Initialize the codegen class"""
        self.opt_config = opt_config
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
        self.calibration = calibration
        self.qf = qf

    def _get_max_shape(
            self, node, acceleration, max_buffer_shape, max_buffer_size):
        if node.get_buffer_size(acceleration) is not None:
            size = node.get_buffer_size(acceleration)
            new_size = 1
            for s in size:
                new_size *= s

            if max_buffer_size < new_size:
                max_buffer_size = new_size
                max_buffer_shape = size

            return True, max_buffer_shape, max_buffer_size

        return False, max_buffer_shape, max_buffer_size

    def gen(
        self, dataset_size, max_dma_size, print_freq=100, loc="hi",
        calibration=False, intermittent=False, intermittent_args=None,
        hifram_func=0
    ):
        """Generate the code"""
        if intermittent:
            jinja_dir = os.path.join(JINJA_DIR, 'intermittent')
        else:
            jinja_dir = os.path.join(JINJA_DIR, 'continuous')

        # Create the include directory
        if not os.path.exists(os.path.join(self.src_dir, "include")):
            os.makedirs(os.path.join(self.src_dir, "include"))

        # Generate the main file
        maingen(
            self.src_dir, self.graph, self.opt_config["name"], dataset_size,
            jinja_dir, print_freq=print_freq, add_timer=self.add_timer,
            debug=self.debug, label=self.debug_input_label,
            calibration=self.calibration, intermittent_args=intermittent_args
        )

        # Generate the model file
        modelgen(
            self.src_dir, self.graph, self.debug, self.calibration, jinja_dir,
            hifram_func
        )

        # Generate the weight and bias files
        params_dir = os.path.join(self.src_dir, "params")
        if not os.path.exists(params_dir):
            os.makedirs(params_dir)
            os.makedirs(os.path.join(params_dir, "include"))
        flt_sizes = weightgen(params_dir, self.graph, self.qf, loc=loc)

        # check if the model needs extra buffer
        has_extra_buffer = False
        max_size = 0
        max_shape = None
        for n in self.graph.get_hidden_layers():
            node = self.graph.node_list[n]
            acceleration = None
            if calibration:
                acceleration = self.graph.node_list[n].get_calibration()
                if acceleration is not None:
                    flag, max_shape, max_size = self._get_max_shape(
                        node, acceleration, max_shape, max_size)
                    has_extra_buffer |= flag
                    continue

            flag, max_shape, max_size = self._get_max_shape(
                node, acceleration, max_shape, max_size)
            has_extra_buffer |= flag

        # Generate the buffer files
        buffer_dir = os.path.join(self.src_dir, "buffer")
        if not os.path.exists(buffer_dir):
            os.makedirs(buffer_dir)
            os.makedirs(os.path.join(buffer_dir, "include"))

        intermittent_verify = False
        if intermittent_args is not None:
            intermittent_verify = intermittent_args["verify"]

        arrgen(
            buffer_dir, self.graph, max_shape, self.qf, loc=loc,
            debug_input=self.debug_input, calibration=self.calibration,
            intermittent_verify=intermittent_verify
        )

        # Generate the layer code
        layer_dir = os.path.join(self.src_dir, "layers")
        if not os.path.exists(layer_dir):
            os.makedirs(layer_dir)
            os.makedirs(os.path.join(layer_dir, "include"))
        utilsgen(
            layer_dir, self.opt_config, flt_sizes, self.qf, jinja_dir,
            max_dma_size
        )
        layergen(
            layer_dir, self.graph, self.opt_config, self.qf,
            self.debug, self.calibration, jinja_dir, hifram_func - 1
        )

        # Generate the makefile
        makefilegen(
            self.code_dir, self.graph, has_extra_buffer, self.opt_config,
            self.calibration, jinja_dir,
            intermittent_verify=intermittent_verify
        )

        if intermittent:
            intermittentgen(layer_dir, self.graph, jinja_dir)

    def print_config(self):
        """Print the configuration"""
        pprint.pprint(dict(self.opt_config.items()))

    def setup_debug_info(self, input_arr, label, debug):
        """Set up input array and label"""
        self.debug = debug
        self.debug_input = input_arr
        self.debug_input_label = label
