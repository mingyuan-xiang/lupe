""""
Generates the main file for the MSP430 project.

The template for the main file is in the jinja_templates directory.
"""

import os

from .helpers import jinja_gen

def maingen(
        code_dir, graph, config_name, dataset_size, jinja_dir,
        print_freq=100, add_timer=True, debug=False, label=0, calibration=False,
        intermittent_args=None
    ):
    """Generate the main file using jinja template"""
    template_path = os.path.join(jinja_dir, "main.c.jinja")

    model_name = graph.name
    nodes = graph.get_hidden_layers()

    if intermittent_args is not None:
        params = {
            "model_name": model_name,
            "repeat": intermittent_args["repeat"],
            "bounds" : intermittent_args["bounds"],
            "verify" : intermittent_args["verify"],
            "last_layer" : nodes[-1],
        }
    else:
        params = {
            "model_name": model_name,
            "config_name" : config_name,
            "dataset_size": dataset_size,
            "add_timer": add_timer,
            "print_freq": print_freq,
            "debug" : debug,
            "label" : label,
            "calibration" : calibration,
        }

    jinja_gen((template_path, params), None, "main", code_dir)
