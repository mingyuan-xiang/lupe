import sys
import os

import numpy as np

file_dir = os.path.dirname(os.path.realpath(__file__))
matrixgen_path = file_dir
matrixgen_path += '/../../../codegen/msp430/'
sys.path.append(matrixgen_path)

from matrixgen import (
    Matrix, save, gen_header_data, gen_header_includes,
    gen_c, gen_c_data_struct, gen_c_data
)

def mul(t):
    result = 1
    for num in t:
        result *= num

    return result



def gen(name, shape, a_shape, is_random):
    file_name = name


    h_code = gen_header_includes(file_name)
    c_code = '#include <' + 'include/' + file_name + '.h>\n\n'

    if is_random:
        num = np.random.uniform(low=-1000.0/32767, high=1000.0/32767,  size=a_shape)
    else:
        num = np.zeros(a_shape)

    if shape == a_shape:
        mat = Matrix(name, num, False, loc='hi')

        h_code += gen_header_data(mat)
        c_code += gen_c_data_struct(mat, None)
        c_code += gen_c_data(mat, False)
    else:
        data_mat = Matrix(name + '_data', num, False, loc='hi')
        mat = Matrix(name, np.zeros(shape), False, loc='hi')

        h_code += gen_header_data(mat)
        h_code += gen_header_data(data_mat)
        c_code += gen_c_data_struct(mat, name + '_data')
        c_code += gen_c_data_struct(data_mat, None)
        c_code += gen_c_data(data_mat, False)

    h_code += "#endif\n"

    with open(file_dir + '/include/' + file_name + '.h', "w", encoding="utf-8") as f:
        f.write(h_code)
    with open(file_dir + '/' + file_name + '.c', "w", encoding="utf-8") as f:
        f.write(c_code)

k = 3
pad = 1
in_shape = (1, 3, 82, 82)
out_shape = (1, 8, 40, 40)
image_shape = (in_shape[0], in_shape[1], in_shape[2] - 2 * pad, in_shape[3] - 2 * pad)
# weight_shape = (out_shape[1], 1, k, k)
weight_shape = (out_shape[1], in_shape[1], k, k)
bias_shape = (1, out_shape[1])


image_shape = (in_shape[0], in_shape[1], in_shape[2] - 2 * pad, in_shape[3] - 2 * pad)
image_shape = (in_shape[0], in_shape[1], in_shape[2] - 2 * pad, in_shape[3] - 2 * pad)
image_shape = (in_shape[0], in_shape[1], in_shape[2] - 2 * pad, in_shape[3] - 2 * pad)

actual_shape = in_shape if mul(in_shape) > mul(out_shape) else out_shape

gen('image', image_shape, image_shape, True)
gen('input', in_shape, actual_shape, True)
gen('output', out_shape, actual_shape, False)
gen('output_exp', out_shape, actual_shape, False)
gen('weight', weight_shape, weight_shape, True)
gen('bias', bias_shape, bias_shape, True)
