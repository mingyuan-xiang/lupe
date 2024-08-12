"""The layer node in the graph"""

from abc import ABC, abstractmethod
from .layer_utils import name_conversion

class LupeLayer(ABC):
    """
    The basic layer node in the graph
    """
    def __init__(
        self, node, model, node_list, opt_config, io=None, qf_offset=0):
        """Initialize the layer

        Args:
            node: The ONNX node
            model: The ONNX model
            node_list: The dictionary of nodes
            opt_config: optimization configuration
            io: Indicate if the layer is an input or output (None if other)
            qf_offset: Fixed-point offset for weights
        """
        self.name = self._get_name(node)
        self.opt_config = opt_config
        self.qf_offset = qf_offset

        if io is None:
            self.input_size = self._get_input_size(node, model)
            self.output_size = self._get_output_size(node, model)
        elif io == "input":
            self.input_size = None
            self.output_size = [
                dim.dim_value for dim in node.type.tensor_type.shape.dim
            ]
        elif io == "output":
            self.input_size = [
                dim.dim_value for dim in node.type.tensor_type.shape.dim
            ]
            self.output_size = None
        else:
            raise ValueError("Invalid io arguments")

        # Get the input and output size before register a node
        self._register(node)
        self._add_weights(node, node_list)

    @abstractmethod
    def _get_name(self, node):
        """Get the name of the layer"""

    @abstractmethod
    def has_weights(self):
        """If the layer has weights"""

    @abstractmethod
    def get_buffer_size(self, acceleration):
        """If the layer needs extra buffer. Return the buffer shape tuple"""

    def flip(self):
        """If the layer should flip the weights"""
        return False

    def transpose(self):
        """If the layer should transpose the weights"""
        return False

    def get_calibration(self):
        """Get the list of acceleration method for calibration"""
        return None

    @abstractmethod
    def get_code(self, name, jinja_dir, opt_config, qf, acceleration):
        """Get the code for the layer"""

    def _register(self, node):
        """Register the layer"""

    def _add_weights(self, node, node_list):
        """Register the weights"""
        if self.has_weights():
            if node_list is None:
                raise ValueError("node_list is None")
            if node.op_type in ("Conv", "Gemm"):
                weight_name = name_conversion(node.input[1])
                self.weight = node_list[weight_name]
                bias_name = name_conversion(node.input[2])
                self.bias = node_list[bias_name]

        # register min and max for clip
        if "Clip" in node.name:
            if node_list is None:
                raise ValueError("node_list is None")
            min_name = name_conversion(node.input[1])
            self.min = node_list[min_name].value
            max_name = name_conversion(node.input[2])
            self.max = node_list[max_name].value

    def _get_input_size(self, node, model):
        """Set the input size"""
        if model is None or len(node.input) < 1:
            return None
        # Assume the input is the first element in inputs
        input_name = node.input[0]
        for v in model.graph.value_info:
            if v.name == input_name:
                tensor_shape = v.type.tensor_type.shape
                return tuple(dim.dim_value for dim in tensor_shape.dim)

        # This is the first layer, and the model input is technically not a node
        node = model.graph.input[0]
        return tuple(dim.dim_value for dim in node.type.tensor_type.shape.dim)

    def _get_output_size(self, node, model):
        """Set the output size"""
        if model is None or len(node.output) < 1:
            return None
        # Assume the output is the first element in outputs
        output_name = node.output[0]
        for v in model.graph.value_info:
            if v.name == output_name:
                tensor_shape = v.type.tensor_type.shape
                return tuple(dim.dim_value for dim in tensor_shape.dim)

        # This is the last layer, and the model output is technically not a node
        node = model.graph.output[0]
        return tuple(dim.dim_value for dim in node.type.tensor_type.shape.dim)

    def print(self):
        """Print the layer"""
        print(str(self))
