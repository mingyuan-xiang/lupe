"""Input and output layer"""

from .layer import LupeLayer
from .layer_utils import name_conversion

class InOut(LupeLayer):
    """Input and output layer"""

    def _register(self, node):
        """Register the layer
        
        Args:
            node: ONNX node
        """
        shape = [dim.dim_value for dim in node.type.tensor_type.shape.dim]
        self.shape = tuple(shape)

    def __str__(self):
        return f"{self.name}: InOut(shape={self.shape})"

    def _get_name(self, node):
        """Get the name of the layer"""
        return name_conversion(node.name)

    def has_weights(self):
        """If the layer has weights"""
        return False

    def get_buffer_size(self, acceleration):
        """If the layer needs extra buffer. Return the buffer shape tuple"""
        return None

    def get_code(self, name, jinja_dir, opt_config, qf, acceleration):
        """This will never get called"""
        raise NotImplementedError
