""""
Generates the main file for the MSP430 project.

The template for the main file is in the jinja_templates directory.
"""

import os

from jinja2 import Template

from . import JINJA_DIR

def maingen(
        code_dir, model_name, dataset_size, print_freq=100, add_timer=True
    ):
    """Generate the main file using jinja template"""
    template_path = os.path.join(JINJA_DIR, "main.jinja")

    with open(template_path, "r", encoding="utf-8") as file:
        template = file.read()
        params = {
            "model_name": model_name,
            "dataset_size": dataset_size,
            "add_timer": add_timer,
            "print_freq": print_freq,
        }

        j_template = Template(template)

        main_str = j_template.render(params)

    with open(os.path.join(code_dir, "main.c"), "w", encoding="utf-8") as file:
        file.write(main_str)
