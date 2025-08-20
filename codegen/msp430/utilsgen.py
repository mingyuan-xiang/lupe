# SPDX-License-Identifier: AGPL-3.0-or-later
# Copyright (C) 2025 Mingyuan Xiang

"""Generates common helper functions for each layer"""

import os

from .helpers import jinja_gen

def utilsgen(code_dir, opt_config, flt_sizes, qf, jinja_dir, max_dma_size):
    """Generates common helper functions for each layer
    
    Args:
        code_dir: The directory to save the code
        opt_config: The optimization configuration
    """
    # LEA size has to be multiple of 2
    if opt_config["lea_size"] % 2:
        raise ValueError('LEA size has to be multiple of 2')

    for s in flt_sizes:
        if opt_config["lea_size"] < s:
            raise ValueError(
                'LEA filter array must be greater than the kernel matrix size'
            )

    # utils header
    header_template_path = os.path.join(jinja_dir, "utils.h.jinja")
    header_params = {
        "max_dma_size" : max_dma_size,
        "dma_opt": opt_config["dma_opt"],
        "lea_size" : opt_config["lea_size"],
        "lea_opt" : opt_config["lea_opt"],
        "qf" : qf,
        "global_mem_buffer" : opt_config["global_mem_buffer"],
    }

    # utils c file
    cfile_template_path = os.path.join(jinja_dir, "utils.c.jinja")
    cfile_params = {
        "dma_opt": opt_config["dma_opt"],
        "lea_opt" : opt_config["lea_opt"],
        "qf" : qf,
        "global_mem_buffer" : opt_config["global_mem_buffer"],
    }

    jinja_gen(
        (cfile_template_path, cfile_params),
        (header_template_path, header_params),
        "utils",
        code_dir
    )
