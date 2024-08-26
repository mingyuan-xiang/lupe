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

file_name = 'random_data'
buffer_name = 'random_data'

h_code = gen_header_includes(file_name)
c_code = '#include <' + 'include/' + file_name + '.h>\n\n'

num = np.random.uniform(low=-1000.0/32767, high=1000.0/32767,  size=(1, 1, 1, 1000))
mat = Matrix('random_data', num, False, loc='hi')
h_code += gen_header_data(mat)
h_code += "#endif\n"
c_code += gen_c_data_struct(mat, None)
c_code += gen_c_data(mat, False)

with open(file_dir + '/include/' + file_name + '.h', "w", encoding="utf-8") as f:
    f.write(h_code)
with open(file_dir + '/' + file_name + '.c', "w", encoding="utf-8") as f:
    f.write(c_code)
