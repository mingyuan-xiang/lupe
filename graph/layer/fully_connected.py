"""Fully connected layer"""

from .layer import LupeLayer
from .layer_utils import get_onnx_attr, name_conversion

class FullyConnected(LupeLayer):
    """FullyConnected layer
    
    A' = transpose(A) if transA else A
    B' = transpose(B) if transB else B
    Compute Y = alpha * A' * B' + beta * C
    https://onnx.ai/onnx/operators/onnx__Gemm.html
    
    """
    def _register(self, node):
        """Register the layer"""
        # alpha
        alpha = get_onnx_attr(node, "alpha")
        self.alpha = alpha.f if alpha else None
        # beta
        beta = get_onnx_attr(node, "beta")
        self.beta = beta.f if beta else None
        # transA
        trans_a = get_onnx_attr(node, "transA")
        self.trans_a = trans_a is None or trans_a.i == 0
        # transB
        trans_b = get_onnx_attr(node, "transB")
        self.trans_b = trans_b is None or trans_b.i == 0

    def __str__(self):
        s = f"{self.name}: Convolution2D("
        if self.alpha:
            s += f"alpha={self.alpha}, "
        if self.beta:
            s += f"beta={self.beta}, "
        s += f"transA={self.trans_a}, "
        s += f"transB={self.trans_b}"
        s += ")"

        return s

    def _get_name(self, node):
        """Get the name of the layer"""
        for i in node.input:
            if "weight" in i:
                return name_conversion(i)[:-len("_weight")]

        raise NameError(
            f"The convolution layer {node.name} doesn't have weights"
        )

    def has_weights(self):
        """If the layer has weights"""
        return True
