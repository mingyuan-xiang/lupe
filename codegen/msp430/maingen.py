""""
Generates the main file for the MSP430 project.

The template for the main file is in the jinja_templates directory.
"""

import os

from . import JINJA_DIR
from .helpers import jinja_gen

def maingen(
        code_dir, model_name, dataset_size, print_freq=100,
        add_timer=True, debug=False, label=0
    ):
    """Generate the main file using jinja template"""
    template_path = os.path.join(JINJA_DIR, "main.c.jinja")
    params = {
        "model_name": model_name,
        "dataset_size": dataset_size,
        "add_timer": add_timer,
        "print_freq": print_freq,
        "debug" : debug,
        "label" : label,
    }

    jinja_gen((template_path, params), None, "main", code_dir)
