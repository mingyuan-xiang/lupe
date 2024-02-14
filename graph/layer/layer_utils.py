"""The layer node in the graph"""

def get_onnx_attr(node, s):
    """Get the ONNX attribute
    
    Args:
        node: The ONNX node
        s: The attribute name
    """
    return next(
        (attr for attr in node.attribute if attr.name == s), None
    )
