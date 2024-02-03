"""Input and output layer"""

from layer import LupeLayer

class InOut(LupeLayer):
    """Input and output layer"""

    def __init__(self, name, dims):
        """Initialize the input or output layer.
        
        Instead of using the ONNX node, the input and output layers are just
        lists to represent the input and output shapes.
        """
        super().__init__(name, None)
        self.dims = dims
