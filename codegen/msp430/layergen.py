"""Generates code for each layer of the graph"""

import os

from . import JINJA_DIR, INDENT
from .helpers import jinja_gen

def layergen(code_dir, graph, opt_config, qf):
    """Generates code for each layer of the graph
    
    Args:
        code_dir: The directory to save the code
        graph: The graph object
        opt_config: The optimization configuration
    """
    nodes = graph.get_hidden_layers()
    for n in nodes:
        node = graph.node_list[n]
        # header
        header_template_path = os.path.join(JINJA_DIR, "layer.h.jinja")
        header_params = {
            "layer_name": node.name,
            "has_weights" : node.has_weights(),
        }

        jinja_dir = os.path.join(JINJA_DIR, "layers")
        # c file
        cfile_template_path = os.path.join(JINJA_DIR, "layer.c.jinja")
        cfile_params = {
            "layer_name": node.name,
            "code" : node.get_code(jinja_dir, opt_config, qf)
        }

        jinja_gen(
            (cfile_template_path, cfile_params),
            (header_template_path, header_params),
            node.name,
            code_dir
        )
