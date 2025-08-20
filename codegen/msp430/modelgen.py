# SPDX-License-Identifier: AGPL-3.0-or-later
# Copyright (C) 2025 Mingyuan Xiang

""""
Generates the model file for the MSP430 project.

The template for the model file is in the jinja_templates directory.
"""

import os

from .helpers import jinja_gen

def modelgen(code_dir, graph, debug, calibration, jinja_dir, hifram_func):
    """Generate the model using jinja template"""
    model_name = graph.name

    # model header
    header_template_path = os.path.join(jinja_dir, "model.h.jinja")
    header_params = {
        "hifram" : hifram_func > 0,
        "model_name": model_name
    }

    # model c file
    cfile_template_path = os.path.join(jinja_dir, "model.c.jinja")
    nodes = graph.get_hidden_layers()
    nodes_dic = []
    calibration_list = {}
    for n in nodes:
        d = {
            "name" : n,
            "has_weights" : graph.node_list[n].has_weights()
        }
        if graph.node_list[n].has_weights():
            d["input_name"] = graph.node_list[n].input
            d["weight_name"] = graph.node_list[n].weight.name
            d["bias_name"] = graph.node_list[n].bias.name

        if graph.node_list[n].get_calibration() is not None:
            calibration_list[n] = [graph.node_list[n].get_calibration()]
        else:
            calibration_list[n] = []

        nodes_dic.append(d)

    cfile_params = {
        "model_name": model_name,
        "layer_list": nodes_dic,
        "last_layer": nodes[-1],
        "debug" : debug,
        "calibration" : calibration,
        "calibration_list" : calibration_list,
        "has_add_buffer" : False,
    }

    if len(graph.add_buffer_list) > 0:
        inverted_add_buffer_list = {
            v : k for k, v in graph.add_buffer_list.items()
        }
        cfile_params["has_add_buffer"] = True
        cfile_params["add_buffer_list"] = inverted_add_buffer_list

    jinja_gen(
        (cfile_template_path, cfile_params),
        (header_template_path, header_params),
        model_name,
        code_dir
    )
