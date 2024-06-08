"""Helper functions for the layer module"""

def get_onnx_attr(node, s):
    """Get the ONNX attribute
    
    Args:
        node: The ONNX node
        s: The attribute name
    """
    return next(
        (attr for attr in node.attribute if attr.name == s), None
    )

def name_conversion(name):
    """Convert the name to a valid python variable name"""
    n = name.replace(".", "_").replace("-", "_").replace("/", "_").replace(":", "_")
    if n[0] == "_":
        n = n[1:]
    return n
