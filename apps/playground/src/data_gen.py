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

def gen(name, num):
    file_name = name

    h_code = gen_header_includes(file_name)
    c_code = '#include <' + 'include/' + file_name + '.h>\n\n'

    mat = Matrix(name, num, False, loc='hi')
    h_code += gen_header_data(mat)
    h_code += "#endif\n"
    c_code += gen_c_data_struct(mat, None)
    c_code += gen_c_data(mat, False)

    with open(file_dir + '/include/' + file_name + '.h', "w", encoding="utf-8") as f:
        f.write(h_code)
    with open(file_dir + '/' + file_name + '.c', "w", encoding="utf-8") as f:
        f.write(c_code)

pad = 1
in_shape = (1, 1, 51, 14)
image_shape = (in_shape[0], in_shape[1], in_shape[2] - 2 * pad, in_shape[3] - 2 * pad)
num = np.random.uniform(low=-1000.0/32767, high=1000.0/32767,  size=image_shape)
gen('image', num)
num = np.zeros(in_shape)
gen('input', num)
out_shape = (1, 64, 21, 6)
out_ch = out_shape[1]
num = np.zeros(out_shape)
gen('output_exp', num)
num = np.zeros(out_shape)
gen('output', num)
num = np.random.uniform(low=-1000.0/32767, high=1000.0/32767,  size=(64, 1, 10, 4))
gen('weight', num)
num = np.random.uniform(low=-1000.0/32767, high=1000.0/32767,  size=(1, out_ch))
gen('bias', num)
