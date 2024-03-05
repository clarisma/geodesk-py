import sys
import os

classname = sys.argv[1]
is_py = classname.startswith("Py")

fileheader = \
    '// Copyright (c) 2024 Clarisma / GeoDesk contributors\n' \
    '// SPDX-License-Identifier: LGPL-3.0-only\n'

pymethods = [ 
    ("PyObject*", "call", ", PyObject* args, PyObject* kwargs"),
    ("void", "dealloc", ""),
    ("PyObject*", "getattro", ", PyObject *attr"),
    ("Py_hash_t", "hash", ""),
    ("PyObject*", "iter", ""),
    ("PyObject*", "next", ""),
    ("PyObject*", "repr", ""),
    ("PyObject*","richcompare", ", PyObject* other, int op"),
    ("PyObject*", "str", ""),
]

def write_py_decl(f, method):
    ret, name, param = method
    f.write(f'\tstatic {ret} {name}({classname}* self{param});\n')

def write_py_impl(f, method):
    ret, name, param = method
    f.write(f'{ret} {classname}::{name}({classname}* self{param})\n')
    f.write('{\n\t// TODO\n')
    if ret != 'void':
        if ret == 'int':
            f.write('\treturn 0;\n')
        if ret == 'Py_hash_t':
            f.write('\treturn 0;\n')
        else:
            f.write('\tPy_RETURN_NONE;\n')
    f.write('}\n\n')

# Create the files
with open(f"{classname}.h", 'x') as f:       # "x" means don't overwrite existing
    f.write(fileheader)
    f.write('\n#pragma once\n')
    if is_py:
        f.write('#include <Python.h>\n')
        f.write('#include <structmember.h>\n\n')
    else:
        f.write('\n');
    f.write(f'class {classname}\n')
    f.write('{\npublic:\n')
    if is_py:
        f.write('\tPyObject_HEAD\n\n')
        f.write('\tstatic PyTypeObject TYPE;\n')
        f.write('\tstatic PyMethodDef METHODS[];\n')
        f.write('\tstatic PyMemberDef MEMBERS[];\n')
        f.write('\tstatic PyGetSetDef GETSET[];\n')
        f.write('\tstatic PyNumberMethods NUMBER_METHODS;\n')
        f.write('\tstatic PySequenceMethods SEQUENCE_METHODS;\n')
        f.write('\tstatic PyMappingMethods MAPPING_METHODS;\n\n')
        
        f.write('\tstatic PyObject* create();\n')
        for m in pymethods:
            write_py_decl(f, m)
    else:
        f.write(f'\t{classname}();\n')

    f.write('};\n')

with open(f"{classname}.cpp", 'x') as f:        # "x" means don't overwrite existing
    f.write(fileheader)
    f.write(f'\n#include "{classname}.h"\n\n')
    if is_py:
        for m in pymethods:
            write_py_impl(f, m)

        f.write(f'PyMethodDef {classname}::METHODS[] =\n')
        f.write('{\n')
        f.write(f'\t{{"save", (PyCFunction)save, METH_VARARGS, "Saves the file" }},\n')
        f.write('\t{ NULL, NULL, 0, NULL },\n')
        f.write('};\n\n')
        f.write(f'PyMemberDef {classname}::MEMBERS[] =\n')
        f.write('{\n')
        f.write(f'\t{{"first", T_OBJECT_EX, offsetof({classname}, first), 0, "first name"}},\n')
        f.write('\t{ NULL, 0, 0, 0, NULL },\n')
        f.write('};\n\n')
        f.write(f'PyGetSetDef {classname}::GETSET[] =\n')
        f.write('{\n')
        f.write('\t{ "value", (getter)get_value, (setter)set_value, "Description of value", NULL },\n')
        f.write('\t{ NULL, NULL, NULL, NULL, NULL },\n')
        f.write('};\n\n')
        
        f.write(f'PyNumberMethods {classname}::NUMBER_METHODS =\n')
        f.write('{\n};\n\n')
        f.write(f'PySequenceMethods {classname}::SEQUENCE_METHODS =\n')
        f.write('{\n};\n\n')
        f.write(f'PyMappingMethods {classname}::MAPPING_METHODS =\n')
        f.write('{\n};\n\n')

        f.write(f'PyTypeObject {classname}::TYPE =\n')
        f.write('{\n')
        f.write(f'\t.tp_name = "geodesk.{classname[2:]}",\n')
        f.write(f'\t.tp_basicsize = sizeof({classname}),\n')
        f.write(f'\t.tp_dealloc = (destructor)dealloc,\n')
        f.write(f'\t.tp_repr = (reprfunc)repr,\n')
        f.write(f'\t.tp_as_number = &NUMBER_METHODS,\n')
        f.write(f'\t.tp_as_sequence = &SEQUENCE_METHODS,\n')
        f.write(f'\t.tp_as_mapping = &MAPPING_METHODS,\n')
        f.write(f'\t.tp_hash = (hashfunc)hash,\n')
        f.write(f'\t.tp_call = (ternaryfunc)call,\n')
        f.write(f'\t.tp_str = (reprfunc)str,\n')
        f.write(f'\t.tp_getattro = (getattrofunc)getattro,\n')
        f.write(f'\t.tp_flags = Py_TPFLAGS_DEFAULT, // | Py_TPFLAGS_DISALLOW_INSTANTIATION,\n')
        f.write(f'\t.tp_doc = "{classname[2:]} objects",\n')
        f.write(f'\t.tp_richcompare = (richcmpfunc)richcompare,\n')
        f.write(f'\t.tp_iter = (getiterfunc)iter,\n')
        f.write(f'\t.tp_iternext = (iternextfunc)next,\n')
        f.write(f'\t.tp_methods = METHODS,\n')
        f.write(f'\t.tp_members = MEMBERS,\n')
        f.write(f'\t.tp_getset = GETSET,\n')
        f.write('};\n')
    else:
        f.write(f'{classname}::{classname}()\n')
        f.write('{\n\n}\n')


print(f"Created {classname}.h and {classname}.cpp")
