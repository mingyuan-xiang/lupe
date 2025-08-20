# SPDX-License-Identifier: AGPL-3.0-or-later
# Copyright (C) 2025 Mingyuan Xiang

import os

from jinja2 import Template

def _gen(t, file_path):
    """Generate code using jinja template
    
    Args:
        t: A tuple containing the path to the template and the parameters,
            e.g. (path, params)
        file_path: The path to save the generated code
    """
    path, params = t
    with open(path, "r", encoding="utf-8") as file:
        template = file.read()
        j_template = Template(template)
        code_str = j_template.render(params)

    with open(os.path.join(file_path), "w", encoding="utf-8") as file:
        file.write(code_str)

def jinja_gen(c_template, h_template, name, code_dir):
    """Generate code using jinja template
    
    Args:
        c_template: A tuple containing the path to the c template and
            the parameters, e.g. (path, params)
        h_template: A tuple containing the path to the h template and
            the parameters, e.g. (path, params)
        name: The name of the file (without the extension)
        code_dir: The directory to save the code
    """
    if h_template is not None:
        header_path = os.path.join(code_dir, "include", name + ".h")
        _gen(h_template, header_path)

    if c_template is not None:
        cfile_path = os.path.join(code_dir, name + ".c")
        _gen(c_template, cfile_path)

def add_indent(s, n):
    """Add n spaces to each line of the string"""
    if s is not None:
        return "\n".join([" " * n + line for line in s.split("\n")])
    return None
