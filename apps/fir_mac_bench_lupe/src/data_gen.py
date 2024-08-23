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

def _list_mul(l):
    """Multiply the elements of a list"""
    result = 1
    for i in l:
        result *= i
    return result

flt_sizes = [3, 5]
channels = [10]
in_sizes = [10]

file_name = 'data'
buffer_name = 'data'

h_code = gen_header_includes(file_name)
c_code = '#include <' + 'include/' + file_name + '.h>\n\n'

buffer_size = (1, 1)

cnt = 0

def get_mat(shape, name, cnt, buffer_size):
    size = buffer_size
    if _list_mul(shape) > _list_mul(size):
        size = shape
    mat = Matrix(
        name + '_' + str(cnt), np.zeros(shape), False, loc='hi'
    )

    h = gen_header_data(mat, gen_array=False)
    c = gen_c_data_struct(mat, buffer_name)

    return size, h, c

for f in flt_sizes:
    for i in in_sizes:
        o = i - f + 1
        for idx in range(len(channels)):
            ic = channels[idx]
            if idx == len(channels) - 1:
                ch = [ic]
            else:
                ch = [ic, channels[idx + 1]]
            for oc in ch:
                w_shape = (oc, ic, f, f)
                b_shape = (1, oc)
                i_shape = (1, ic, i, i)
                o_shape = (1, oc, o, o)

                shape_dict = {
                    'weight' : w_shape,
                    'bias' : b_shape,
                    'input' : i_shape,
                    'output' : o_shape,
                }

                for n in shape_dict:
                    buffer_size, h, c = get_mat(
                        shape_dict[n], n, cnt, buffer_size
                    )
                    h_code += h
                    c_code += c

                cnt += 1

num = np.random.uniform(low=-1000.0/32767, high=1000.0/32767,  size=(10, 10, 28, 28))
mat = Matrix('data', num, False, loc='hi')
h_code += gen_header_data(mat)
h_code += "#endif\n"
c_code += gen_c_data_struct(mat, None)
c_code += gen_c_data(mat, False)

with open(file_dir + '/include/' + file_name + '.h', "w", encoding="utf-8") as f:
    f.write(h_code)
with open(file_dir + '/' + file_name + '.c', "w", encoding="utf-8") as f:
    f.write(c_code)
