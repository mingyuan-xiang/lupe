"""Generates common helper functions for each layer"""

import os

from . import JINJA_DIR
from .helpers import jinja_gen

def utilsgen(code_dir, opt_config):
    """Generates common helper functions for each layer
    
    Args:
        code_dir: The directory to save the code
        opt_config: The optimization configuration
    """
    # utils header
    header_template_path = os.path.join(JINJA_DIR, "utils.h.jinja")
    header_params = {"dma_opt": opt_config["dma"],}

    # utils c file
    cfile_template_path = os.path.join(JINJA_DIR, "utils.c.jinja")
    cfile_params = {}

    jinja_gen(
        (cfile_template_path, cfile_params),
        (header_template_path, header_params),
        "utils",
        code_dir
    )
