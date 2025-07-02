"""Generates pre-allocated arrays for the MSP430."""

import os
import numpy as np

from .matrixgen import (
    Matrix, save, gen_header_data, gen_header_includes,
    gen_c, gen_c_data_struct, gen_c_data
)

def _list_mul(l):
    """Multiply the elements of a list"""
    result = 1
    for i in l:
        result *= i
    return result


def _inputgen(
    code_dir, graph, qf, loc="hi", debug_input=None,
    calibration=False):
    """Generate the input files"""
    input_data = graph.node_list[graph.input_name]
    # force the input size to have dimension 4
    if len(input_data.shape) > 4:
        raise ValueError("Input size is too large")
    while len(input_data.shape) < 4:
        input_data.shape = (1, *input_data.shape)
    if debug_input is None:
        if calibration:
            low = -1.0 / (2**qf)
            high = 1.0 / (2**qf)
            data = np.random.uniform(low, high, input_data.shape)
        else:
            data = np.zeros(input_data.shape)
        input_data = Matrix(input_data.name, data, False, loc=loc)
    else:
        input_data = Matrix(input_data.name, debug_input, False, loc=loc)

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
    c_code = gen_c("buffer", input_file_name)
    c_code += gen_c_data_struct(input_data, None)
    c_code += gen_c_data(input_data, False)
    save(
        c_code,
        os.path.join(code_dir, input_file_name + ".c")
    )

def _outputgen(code_dir, graph, loc="hi"):
    """Generate the input files"""
    last_layer = graph.get_hidden_layers()[-1]
    shape = graph.node_list[last_layer].output_size

    data = Matrix('output', np.zeros(shape), False, loc=loc)

    file_name = 'output'

    # Generate header file
    h_code = gen_header_includes(file_name)
    h_code += gen_header_data(data) + "\n"
    h_code += "#endif\n"
    save(
        h_code,
        os.path.join(code_dir, "include", file_name + ".h")
    )

    # Generate C file
    c_code = gen_c("buffer", file_name)
    c_code += gen_c_data_struct(data, None)
    c_code += gen_c_data(data, False)
    save(
        c_code,
        os.path.join(code_dir, file_name + ".c")
    )

def _extra_buffergen(code_dir, size, loc="hi"):
    """Generate the extra buffer files"""
    shape = list(size)
    while len(shape) < 4:
        shape.insert(0, 1)

    buffer_name = "lupe_buffer"
    buffer_data = Matrix(buffer_name, np.zeros(size), False, loc=loc)

    # Generate header file
    h_code = gen_header_includes(buffer_name)
    h_code += gen_header_data(buffer_data) + "\n"
    h_code += "#endif\n"
    save(
        h_code,
        os.path.join(code_dir, "include", buffer_name + ".h")
    )

    # Generate C file
    c_code = gen_c("buffer", buffer_name)
    c_code += gen_c_data_struct(buffer_data, None)
    c_code += gen_c_data(buffer_data, False)
    save(
        c_code,
        os.path.join(code_dir, buffer_name + ".c")
    )

def _add_buffergen(code_dir, graph, loc="hi"):
    """Generate the add buffer files"""
    file_name = "add_buffer"
    buffer_name = "add_buffer"

    h_code = gen_header_includes(file_name)
    c_code = gen_c("buffer", file_name)

    buffer_size = 0
    for l in graph.add_buffer_list:
        node = graph.node_list[l]
        if _list_mul(node.output_size) > buffer_size:
            buffer_size = _list_mul(node.output_size)

        mat = Matrix(
            node.name + "_add_buffer", np.zeros(node.output_size),
            False, loc=loc
        )

        # Generate header file
        h_code += gen_header_data(mat, gen_array=False) + "\n"
        # Generate C file
        c_code += gen_c_data_struct(mat, buffer_name)

    # Generate header file
    mat = Matrix(buffer_name, np.zeros(buffer_size), False, loc=loc)
    h_code += gen_header_data(mat) + "\n"

    h_code += "#endif\n"
    save(
        h_code,
        os.path.join(code_dir, "include", file_name + ".h")
    )

    # Generate C file
    c_code += gen_c_data_struct(mat, None)
    c_code += gen_c_data(mat, False)
    save(
        c_code,
        os.path.join(code_dir, file_name + ".c")
    )

def _buffergen(code_dir, graph, loc="hi"):
    """Generate the buffer files"""

    file_name = "buffer"
    in_buffer_name = "in_buffer"
    out_buffer_name = "out_buffer"

    h_code = gen_header_includes(file_name)
    c_code = gen_c("buffer", file_name)

    buffer_size = _list_mul(graph.node_list[graph.input_name].shape)
    in_buffer_ptr = in_buffer_name
    out_buffer_ptr = out_buffer_name
    for n in graph.graph:
        # skip the input and output nodes
        if n in (graph.input_name, graph.output_name):
            continue

        if n in graph.no_alter_list:
            in_buffer_ptr, out_buffer_ptr = out_buffer_ptr, in_buffer_ptr

        node = graph.node_list[n]
        if _list_mul(node.input_size) > buffer_size:
            buffer_size = _list_mul(node.input_size)
        if _list_mul(node.output_size) > buffer_size:
            buffer_size = _list_mul(node.output_size)

        in_mat = Matrix(
            node.name + "_in", np.zeros(node.input_size), False, loc=loc
        )
        out_mat = Matrix(
            node.name + "_out", np.zeros(node.output_size), False, loc=loc
        )

        # Generate header file
        h_code += gen_header_data(in_mat, gen_array=False)
        h_code += gen_header_data(out_mat, gen_array=False) + "\n"
        # Generate C file
        c_code += gen_c_data_struct(in_mat, in_buffer_ptr)
        c_code += gen_c_data_struct(out_mat, out_buffer_ptr)

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
    c_code += gen_c_data(in_mat, False)
    c_code += gen_c_data(out_mat, False)
    save(
        c_code,
        os.path.join(code_dir, file_name + ".c")
    )

def arrgen(code_dir, graph, size, qf, loc="hi", debug_input=None,
    calibration=False, intermittent_verify=False):
    """Generate the pre-allocated arrays"""
    _inputgen(code_dir, graph, qf, loc, debug_input, calibration)
    if intermittent_verify:
        _outputgen(code_dir, graph, loc)
    _buffergen(code_dir, graph, loc)
    if len(graph.add_buffer_list) > 0:
        _add_buffergen(code_dir, graph, loc)
    if size is not None:
        _extra_buffergen(code_dir, size, loc=loc)
