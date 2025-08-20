# SPDX-License-Identifier: AGPL-3.0-or-later
# Copyright (C) 2025 Mingyuan Xiang

""""
Generates the Makefile for the MSP430 project.

The template for the Makefile is in the jinja_templates directory.
"""

import os

from jinja2 import Template

def makefilegen(code_dir, graph, has_extra_buffer, opt_config, calibration,
    jinja_dir, intermittent_verify=False):
    """Generate the Makefile using jinja template"""
    template_path = os.path.join(jinja_dir, "makefile.jinja")

    nodes_with_weights = [
        n for n in graph.get_hidden_layers() if graph.node_list[n].has_weights()
    ]
    nodes = []
    for n in graph.get_hidden_layers():
        node = graph.node_list[n]
        if calibration:
            acceleration = graph.node_list[n].get_calibration()
            if acceleration is not None:
                layer = node.name + "_" + acceleration
                nodes.append(layer)
                continue

        nodes.append(n)

    params = {
        "model_name" : graph.name,
        "layer_list" : nodes,
        "layers_with_weights" : nodes_with_weights,
        "has_extra_buffer" : has_extra_buffer,
        "has_add_buffer" : False,
        "adaptive_gen_mem" : opt_config["adaptive_gen_mem"],
        "verify" : intermittent_verify,
    }

    if len(graph.add_buffer_list) > 0:
        params["has_add_buffer"] = True

    with open(template_path, "r", encoding="utf-8") as file:
        template = file.read()
        j_template = Template(template)
        code_str = j_template.render(params)

    file_path = os.path.join(code_dir, "Makefile")
    with open(os.path.join(file_path), "w", encoding="utf-8") as file:
        file.write(code_str)
