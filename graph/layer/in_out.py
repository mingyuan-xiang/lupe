"""Input and output layer"""

from .layer import LupeLayer

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
