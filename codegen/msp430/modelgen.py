""""
Generates the main file for the MSP430 project.

The template for the main file is in the jinja_templates directory.
"""

import os

from jinja2 import Template

from . import JINJA_DIR

def modelgen(code_dir, model_name, graph):
    """Generate the model using jinja template"""
    # model header
    template_path = os.path.join(JINJA_DIR, "model.h.jinja")

    with open(template_path, "r", encoding="utf-8") as file:
        template = file.read()
        params = {
            "model_name": model_name,
        }

        j_template = Template(template)

        main_str = j_template.render(params)

    with open(
        os.path.join(code_dir, "include", "model.h"), "w", encoding="utf-8"
    ) as file:
        file.write(main_str)

    # model c file
    template_path = os.path.join(JINJA_DIR, "model.c.jinja")

    # Skip the first and last nodes (for input and output nodes)
    nodes = list(graph.graph.keys())[1:-1]

    with open(template_path, "r", encoding="utf-8") as file:
        template = file.read()
        params = {
            "model_name": model_name,
            "layer_list": nodes,
            "last_layer": nodes[-1],
        }

        j_template = Template(template)

        main_str = j_template.render(params)

    with open(
        os.path.join(code_dir, "model.c"), "w", encoding="utf-8"
    ) as file:
        file.write(main_str)
