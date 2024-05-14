"""Generates common helper functions for each layer"""

import os

from . import JINJA_DIR
from .helpers import jinja_gen

def utilsgen(code_dir, opt_config, flt_sizes):
    """Generates common helper functions for each layer
    
    Args:
        code_dir: The directory to save the code
        opt_config: The optimization configuration
    """
    # LEA size has to be multiple of 2
    if (opt_config["lea_flt_size"] % 2 or opt_config["lea_src_size"] % 2 or
        opt_config["lea_dst_size"] % 2):
        raise ValueError('LEA size has to be multiple of 2')

    for s in flt_sizes:
        if opt_config["lea_flt_size"] < s:
            raise ValueError(
                'LEA filter array must be greater than the kernel matrix size'
            )

    # utils header
    header_template_path = os.path.join(JINJA_DIR, "utils.h.jinja")
    header_params = {
        "dma_opt": opt_config["dma_opt"],
        "lea_flt_size" : opt_config["lea_flt_size"],
        "lea_src_size" : opt_config["lea_src_size"],
        "lea_dst_size" : opt_config["lea_dst_size"]
    }

    # utils c file
    cfile_template_path = os.path.join(JINJA_DIR, "utils.c.jinja")
    cfile_params = {}

    jinja_gen(
        (cfile_template_path, cfile_params),
        (header_template_path, header_params),
        "utils",
        code_dir
    )
