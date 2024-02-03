"""Fully connected layer"""

from layer import LupeLayer, get_onnx_attr

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
        self.trans_a = trans_a.i if trans_a else None
        # transB
        trans_b = get_onnx_attr(node, "transB")
        self.trans_b = trans_b.i if trans_b else None
