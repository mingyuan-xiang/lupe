""""
Generates the Makefile for the MSP430 project.

The template for the Makefile is in the jinja_templates directory.
"""

import os

from jinja2 import Template

from . import JINJA_DIR

def makefilegen(code_dir):
    """Generate the Makefile using jinja template"""
    template_path = os.path.join(JINJA_DIR, "makefile.jinja")
    params = {}

    with open(template_path, "r", encoding="utf-8") as file:
        template = file.read()
        j_template = Template(template)
        code_str = j_template.render(params)

    file_path = os.path.join(code_dir, "Makefile")
    with open(os.path.join(file_path), "w", encoding="utf-8") as file:
        file.write(code_str)
