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


num = np.random.uniform(low=-1000.0/32767, high=1000.0/32767,  size=(1, 6, 14, 14))
gen('input', num)
out_shape = (1, 16, 10, 10)
num = np.zeros(out_shape)
gen('output_exp', num)
num = np.zeros(out_shape)
gen('output', num)
num = np.random.uniform(low=-1000.0/32767, high=1000.0/32767,  size=(16, 6, 5, 5))
gen('weight', num)
