import sys
import os

import numpy as np

file_dir = os.path.dirname(os.path.realpath(__file__))
matrixgen_path = file_dir
matrixgen_path += '/../../codegen/msp430/'
sys.path.append(matrixgen_path)

from matrixgen import (
    Matrix, save, gen_header_data, gen_header_includes,
    gen_c, gen_c_data_struct, gen_c_data
)

# file_name = 'input'
file_name = 'output1'

c_code = f'#include <buffer/include/{file_name}.h>\n\n'

num = np.random.uniform(low=-1000.0/32767, high=1000.0/32767,  size=(1, 6, 14, 14))
mat = Matrix(file_name, num, False, loc='hi')
c_code += gen_c_data_struct(mat, None)
c_code += gen_c_data(mat, False)

with open(file_name + '.c', "w", encoding="utf-8") as f:
    f.write(c_code)
