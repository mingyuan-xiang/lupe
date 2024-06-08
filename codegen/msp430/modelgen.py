""""
Generates the model file for the MSP430 project.

The template for the model file is in the jinja_templates directory.
"""

import os

from . import JINJA_DIR
from .helpers import jinja_gen

def modelgen(code_dir, graph, debug):
    """Generate the model using jinja template"""
    model_name = graph.name

    # model header
    header_template_path = os.path.join(JINJA_DIR, "model.h.jinja")
    header_params = {"model_name": model_name}

    # model c file
    cfile_template_path = os.path.join(JINJA_DIR, "model.c.jinja")
    nodes = graph.get_hidden_layers()
    nodes_dic = []
    for n in nodes:
        d = {
            "name" : n,
            "has_weights" : graph.node_list[n].has_weights()
        }
        if graph.node_list[n].has_weights():
            d["input_name"] = graph.node_list[n].input
            d["weight_name"] = graph.node_list[n].weight.name
            d["bias_name"] = graph.node_list[n].bias.name

        nodes_dic.append(d)
    cfile_params = {
        "model_name": model_name,
        "layer_list": nodes_dic,
        "last_layer": nodes[-1],
        "debug" : debug,
        "has_add_buffer" : False,
    }

    if len(graph.add_buffer_list) > 0:
        cfile_params["has_add_buffer"] = True
        cfile_params["add_buffer_list"] = graph.add_buffer_list

    jinja_gen(
        (cfile_template_path, cfile_params),
        (header_template_path, header_params),
        model_name,
        code_dir
    )
