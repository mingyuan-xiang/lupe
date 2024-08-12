"""
A script to generate header files for matrices.
Ignore sparse matrices for now.
Reference: https://github.com/CMUAbstract/SONIC/blob/master/scripts/gen_headers.py
"""

import os

import numpy as np

from .matrixgen import (
    Matrix, save, gen_header_data, gen_header_includes,
    gen_c, gen_c_data_struct, gen_c_data
)

def weightgen(code_dir, graph, qf, loc="hi"):
    """
    Generate the weight and bias files
    Return:
        The kernel matrix size
    """
    flt_sizes = []
    for n in graph.graph:
        node = graph.node_list[n]
        if node.has_weights():
            param_file_name = node.name

            if node.transpose():
                w = node.weight.data.T
            else:
                w = node.weight.data

            weight = Matrix(
                node.weight.name, w / (2 ** node.weight.qf), False, loc=loc
            )
            bias = Matrix(
                node.bias.name, node.bias.data / (2 ** qf), False, loc=loc
            )
            param_file_name = node.name

            # Generate header file
            h_code = gen_header_includes(param_file_name)
            h_code += gen_header_data(weight) + "\n"
            h_code += gen_header_data(bias)
            h_code += "\n#endif\n"
            save(
                h_code,
                os.path.join(code_dir, "include", param_file_name + ".h")
            )

            # Generate C file
            c_code = gen_c("params", param_file_name)
            c_code += gen_c_data_struct(weight, None)
            c_code += gen_c_data_struct(bias, None)
            c_code += gen_c_data(weight, node.flip())
            c_code += gen_c_data(bias, False)

            save(
                c_code,
                os.path.join(code_dir, param_file_name + ".c")
            )

            if "conv" in node.name:
                flt_sizes.append(node.weight.data[0,0,:,:].size)

    return flt_sizes
