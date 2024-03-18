""""
Generates the model file for the MSP430 project.

The template for the model file is in the jinja_templates directory.
"""

import os

from . import JINJA_DIR
from .helpers import jinja_gen

def modelgen(code_dir, model_name, graph):
    """Generate the model using jinja template"""
    # model header
    header_template_path = os.path.join(JINJA_DIR, "model.h.jinja")
    header_params = {"model_name": model_name}

    # model c file
    cfile_template_path = os.path.join(JINJA_DIR, "model.c.jinja")
    nodes = graph.get_hidden_layers()
    nodes_dic = [{
        "name" : n,
        "has_weights" : graph.node_list[n].has_weights()
    } for n in nodes]
    cfile_params = {
        "model_name": model_name,
        "layer_list": nodes_dic,
        "last_layer": nodes[-1],
    }

    jinja_gen(
        (cfile_template_path, cfile_params),
        (header_template_path, header_params),
        model_name,
        code_dir
    )
