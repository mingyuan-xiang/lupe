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

file_name = 'data'
buffer_name = 'data'

h_code = gen_header_includes(file_name)
c_code = '#include <' + 'include/' + file_name + '.h>\n\n'

num = np.random.uniform(low=-1000.0/32767, high=1000.0/32767,  size=(1, 6, 14, 14))
in_mat = Matrix('input', num, False, loc='hi')
num = np.random.uniform(low=-1000.0/32767, high=1000.0/32767,  size=(16, 6, 5, 5))
w_mat = Matrix('weight', num, False, loc='hi')
out_mat = Matrix('output', np.zeros((1, 16, 10, 10)), False, loc='hi')

h_code += gen_header_data(in_mat, gen_array=False)
h_code += gen_header_data(w_mat, gen_array=False)
h_code += gen_header_data(out_mat, gen_array=False)

c_code += gen_c_data_struct(in_mat, None)
c_code += gen_c_data_struct(w_mat, None)
c_code += gen_c_data_struct(out_mat, None)

h_code += "#endif\n"
c_code += gen_c_data(in_mat, False)
c_code += gen_c_data(w_mat, False)
c_code += gen_c_data(out_mat, False)

with open(file_dir + '/include/' + file_name + '.h', "w", encoding="utf-8") as f:
    f.write(h_code)
with open(file_dir + '/' + file_name + '.c', "w", encoding="utf-8") as f:
    f.write(c_code)
