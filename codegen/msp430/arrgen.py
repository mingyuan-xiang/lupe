"""Generates pre-allocated arrays for the MSP430."""

import os
import numpy as np

from .matrixgen import (
    Matrix, save, gen_header_data, gen_header_includes,
    gen_c, gen_c_data_struct, gen_c_data
)

def _inputgen(code_dir, graph, loc="hi"):
    """Generate the input files"""
    input_data = graph.node_list[graph.input_name]
    # force the input size to have dimension 4
    if len(input_data.shape) > 4:
        raise ValueError("Input size is too large")
    shape = list(input_data.shape)
    while len(input_data.shape) < 4:
        shape.insert(0, 1)
    input_data = Matrix(input_data.name, np.zeros(shape), False, loc=loc)

    input_file_name = input_data.name

    # Generate header file
    h_code = gen_header_includes(input_file_name)
    h_code += gen_header_data(input_data) + "\n"
    h_code += "#endif\n"
    save(
        h_code,
        os.path.join(code_dir, "include", input_file_name + ".h")
    )

    # Generate C file
    c_code = gen_c(input_file_name)
    c_code += gen_c_data_struct(input_data, None)
    c_code += gen_c_data(input_data, True)
    save(
        c_code,
        os.path.join(code_dir, input_file_name + ".c")
    )

def _buffergen(code_dir, graph, loc="hi"):
    """Generate the buffer files"""
    def list_mul(l):
        """Multiply the elements of a list"""
        result = 1
        for i in l:
            result *= i
        return result

    file_name = "buffer"
    in_buffer_name = "in_buffer"
    out_buffer_name = "out_buffer"

    h_code = gen_header_includes(file_name)
    c_code = gen_c(file_name)

    buffer_size = list_mul(graph.node_list[graph.input_name].shape)
    in_buffer_ptr = in_buffer_name
    out_buffer_ptr = out_buffer_name
    for n in graph.graph:
        # skip the input and output nodes
        if n in (graph.input_name, graph.output_name):
            continue
        node = graph.node_list[n]
        if list_mul(node.input_size) > buffer_size:
            buffer_size = list_mul(node.input_size)
        if list_mul(node.output_size) > buffer_size:
            buffer_size = list_mul(node.output_size)

        in_mat = Matrix(node.name, np.zeros(node.input_size), False, loc=loc)
        out_mat = Matrix(node.name, np.zeros(node.output_size), False, loc=loc)

        # Generate header file
        h_code += gen_header_data(in_mat) + "\n"
        h_code += gen_header_data(out_mat) + "\n"
        # Generate C file
        c_code += gen_c_data_struct(in_mat, in_buffer_ptr)
        c_code += gen_c_data_struct(in_mat, out_buffer_ptr)

        in_buffer_ptr, out_buffer_ptr = out_buffer_ptr, in_buffer_ptr

    # Generate header file
    in_mat = Matrix(in_buffer_name, np.zeros(buffer_size), False, loc=loc)
    h_code += gen_header_data(in_mat) + "\n"
    out_mat = Matrix(out_buffer_name, np.zeros(buffer_size), False, loc=loc)
    h_code += gen_header_data(out_mat) + "\n"

    h_code += "#endif\n"
    save(
        h_code,
        os.path.join(code_dir, "include", file_name + ".h")
    )

    # Generate C file
    c_code += gen_c_data_struct(in_mat, None)
    c_code += gen_c_data_struct(out_mat, None)
    c_code += gen_c_data(in_mat, True)
    c_code += gen_c_data(in_mat, True)
    save(
        c_code,
        os.path.join(code_dir, file_name + ".c")
    )

def arrgen(code_dir, graph, loc="hi"):
    """Generate the pre-allocated arrays"""
    _inputgen(code_dir, graph, loc)
    _buffergen(code_dir, graph, loc)
