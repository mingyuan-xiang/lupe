# SPDX-License-Identifier: AGPL-3.0-or-later
# Copyright (C) 2025 Mingyuan Xiang

import os

from .helpers import jinja_gen

def intermittentgen(code_dir, graph, jinja_dir):
    # utils header
    template_path = os.path.join(jinja_dir, "intermittent.h.jinja")

    nodes = []
    for n in graph.get_hidden_layers():
        nodes.append(n)

    params = {
        'layer_list' : nodes,
        'model_name' : graph.name
    }

    jinja_gen(
        None, (template_path, params), "intermittent", code_dir
    )
