""""
Generates the Makefile for the MSP430 project.

The template for the Makefile is in the jinja_templates directory.
"""

import os

from jinja2 import Template

from . import JINJA_DIR

def makefilegen(code_dir, graph, has_extra_buffer):
    """Generate the Makefile using jinja template"""
    template_path = os.path.join(JINJA_DIR, "makefile.jinja")

    nodes = graph.get_hidden_layers()
    nodes_with_weights = [n for n in nodes if graph.node_list[n].has_weights()]
    params = {
        "model_name" : graph.name,
        "layer_list" : nodes,
        "layers_with_weights" : nodes_with_weights,
        "has_extra_buffer" : has_extra_buffer,
    }

    with open(template_path, "r", encoding="utf-8") as file:
        template = file.read()
        j_template = Template(template)
        code_str = j_template.render(params)

    file_path = os.path.join(code_dir, "Makefile")
    with open(os.path.join(file_path), "w", encoding="utf-8") as file:
        file.write(code_str)
