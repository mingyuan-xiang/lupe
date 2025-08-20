# SPDX-License-Identifier: AGPL-3.0-or-later
# Copyright (C) 2025 Mingyuan Xiang

import os

from .helpers import jinja_gen

def _gen(node, name, qf, debug, opt_config, acceleration, code_dir,
    jinja_dir, hifram):
    # header
    header_template_path = os.path.join(jinja_dir, "layer.h.jinja")
    header_params = {
        "layer_name": name,
        "has_weights" : node.has_weights(),
        "hifram" : hifram,
    }

    code_qf = qf
    if node.has_weights():
        code_qf = (qf, node.weight.qf)

    layer_dir = os.path.join(jinja_dir, "layers")
    # c file
    cfile_template_path = os.path.join(jinja_dir, "layer.c.jinja")
    cfile_params = {
        "layer_name": name,
        "debug" : debug,
        "code" : node.get_code(
            name, layer_dir, opt_config, code_qf, acceleration
        ),
        "has_extra_buffer" : node.get_buffer_size(acceleration) is not None
    }

    jinja_gen(
        (cfile_template_path, cfile_params),
        (header_template_path, header_params),
        name,
        code_dir
    )

def layergen(
        code_dir, graph, opt_config, qf, debug, calibration, jinja_dir,
    hifram_func):
    """Generates code for each layer of the graph
    
    Args:
        code_dir: The directory to save the code
        graph: The graph object
        opt_config: The optimization configuration
    """
    nodes = graph.get_hidden_layers()
    cnt = 0

    for n in nodes:
        cnt += 1
        if cnt > hifram_func:
            hifram = False
        else:
            hifram = True

        node = graph.node_list[n]
        if calibration:
            acceleration = graph.node_list[n].get_calibration()
            if acceleration is not None:
                name = node.name + "_" + acceleration
                _gen(
                    node, name, qf, debug, opt_config, acceleration,
                    code_dir, jinja_dir, hifram
                )
                continue

        _gen(
            node, node.name, qf, debug, opt_config, None, code_dir,
            jinja_dir, hifram
        )
