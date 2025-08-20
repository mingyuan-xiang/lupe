# SPDX-License-Identifier: AGPL-3.0-or-later
# Copyright (C) 2025 Mingyuan Xiang

"""
A script to generate header files for matrices.
Ignore sparse matrices for now.
Reference: https://github.com/CMUAbstract/SONIC/blob/master/scripts/gen_headers.py
"""

from colorama import Fore

_header_list = [
    "libfixedAbstract/fixed.h",
    "libmatAbstract/mat.h",
    "libmsp/nv.h",
    "libdsp/DSPLib.h",
]

class Matrix:
    """Generate matrix data in C for MSP430"""
    def __init__(self, name, mat, is_int, loc="hi"):
        """Initialize the matrix object"""
        def float2fix(x):
            if x > 1:
                return str(2**15 - 2)
            elif x < -1:
                return str(-2**15 + 1)
            else:
                return str(int(x * (2**15)))

        self.name = name
        self.mat = mat
        self.type_converter = (
            (lambda x: "(fixed)" + str(x)) if is_int else float2fix
        )
        # Set location to HIFRAM or FRAM
        if loc == "lo":
            self.loc = "__ro_nv"
        else:
            self.loc = "__ro_hinv"

    def gen_mat_data(self, flip):
        """Generate matrix data for the header file in terms of strings"""
        if flip:
            mat = self.mat[..., ::-1]
        else:
            mat = self.mat
        mat_data_list = mat.flatten()
        data = "\t" + self.type_converter(mat_data_list[0])
        # 5 elements per line
        for i in range(1, len(mat_data_list)):
            if i % 9 == 0:
                data += ",\n\t"
            else:
                data += ",\t"
            data += self.type_converter(mat_data_list[i])

        return data


    def gen_mat_var_decl(self, flip):
        """Generate matrix variable declaration."""
        mat_var = self.loc + " _q15 " + self.name
        mat_var += "[] = "

        # Generate matrix data
        mat_var += "{\n" + self.gen_mat_data(flip) + "\n};\n\n"

        return mat_var

    def gen_mat_struct(self, ptr=None):
        """Generate matrix struct declaration."""
        mat_struct = ""
        mat_struct += self.loc + " mat_t " + self.name + "_meta = {\n"
        dims = self.mat.shape
        if len(dims) == 1:
            dims = (1, dims[0])
        mat_struct += (
            "\t.dims = " + str(dims).replace("(", "{").replace(")", "}") + ",\n"
        )
        mat_struct += "\t.len_dims = " + str(len(dims)) + ",\n"
        # Generate strides
        # The stride means the number of elements to skip to get to the next element in
        # the current dimension
        strides = [1]
        for i in range(len(dims)-1, 0, -1):
            s = strides[-1] * dims[i]
            if s > (2**16 - 1):
                print(
                    Fore.RED +
                    f'Stride exceeds limit for uin16_t (got shape {dims}). ' +
                    'Set the stride to zero. There might be a overflow problem.'
                )
                s = 0
            strides.append(s)
        strides.reverse()
        mat_struct += (
            "\t.strides = " +
            str(strides).replace("[", "{").replace("]", "}") + ",\n"
        )

        if ptr is None:
            ptr = self.name

        mat_struct += "\t.data = (fixed *)" + ptr
        mat_struct += "\n"
        mat_struct += "};\n\n"

        return mat_struct

def save(code, loc):
    """Save the header and c files."""
    with open(loc, "w", encoding="utf-8") as f:
        f.write(code)

def gen_header_includes(name):
    """Generate the header file for the includes."""
    h_code = """/*
 * SPDX-License-Identifier: AGPL-3.0-or-later
 * Copyright (C) 2025 Mingyuan Xiang
 */\n
"""
    h_code += "#ifndef " + name.upper() + "_ARRAY_H\n"
    h_code += "#define " + name.upper() + "_ARRAY_H\n\n"
    # add includes
    for header in _header_list:
        h_code += "#include <" + header + ">\n"
    h_code += "\n"

    return h_code

def gen_header_data(mat, gen_array=True):
    """Generate the header file for the matrix data."""
    h_code = ""
    if gen_array:
        h_code += "extern " + mat.loc + " _q15 " + mat.name + "[];\n"
    h_code += "extern " + mat.loc + " mat_t " + mat.name + "_meta;\n"

    return h_code

def gen_c(header_prefix, name):
    """Generate the c file."""
    c_code = """/*
 * SPDX-License-Identifier: AGPL-3.0-or-later
 * Copyright (C) 2025 Mingyuan Xiang
 */\n
"""
    c_code += "#include <" + header_prefix + "/include/" + name + ".h>\n\n"

    return c_code

def gen_c_data_struct(mat, ptr=None):
    """Generate the c file for the matrix data for the struct declaration."""
    c_code = ""
    c_code += mat.gen_mat_struct(ptr)

    return c_code

def gen_c_data(mat, flip):
    """Generate the c file for the matrix data.
    
    Args:
        mat: The matrix object
        flip: Whether to flip the matrix

    Returns:
        The C code for the matrix data
    """

    return mat.gen_mat_var_decl(flip)
