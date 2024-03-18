GCC produces invalid code for this loop:

if (!PyList_Check(list))
{
PyErr_SetString(PyExc_TypeError, "Must be a sequence of strings");
return -1;
}
Py_ssize_t len = PyList_GET_SIZE(list);
//Console::msg("There are %d indexed key entries", len);
printf("There are %zd indexed key entries\n", len);
for (Py_ssize_t i = 0; i < len; i++)
{
//Console::msg("Indexed key entry %d", i);
printf("Indexed key entry %zd\n", i);
// int category = i + 1;
/*
PyObject* item = PyList_GET_ITEM(list, i);
if(item == NULL)
{
printf("NULL!!!!");
//Console::msg("NULL item at index %d!", i);
continue;
}
*/
/*
PyTypeObject* type = Py_TYPE(item);
if (type == &PyUnicode_Type)
{
if (addIndexedKey(item, category) < 0) return -1;
}
else
{
PyObject* tuple = PySequence_Fast(item, "Items must be strings or tuples of strings");
Py_ssize_t tupleLen = PySequence_Fast_GET_SIZE(tuple);
for (Py_ssize_t i2 = 0; i2 < tupleLen; i2++)
{
PyObject* tupleItem = PySequence_Fast_GET_ITEM(tuple, i2);
if (addIndexedKey(tupleItem, category) < 0)
{
Py_DECREF(tuple);
return -1;
}
}
}
*/
}

uildSettings::setIndexedKeys(_object*):
endbr64
push	rbp
push	rbx
sub	rsp, 8
mov	rax, QWORD PTR 8[rsi]
test	BYTE PTR 171[rax], 2
jne	.L20
mov	rdi, QWORD PTR PyExc_TypeError[rip]
lea	rsi, .LC0[rip]
call	PyErr_SetString@PLT
add	rsp, 8
mov	eax, -1
pop	rbx
pop	rbp
ret
.L20:
mov	rdx, QWORD PTR 16[rsi]
mov	edi, 1
xor	eax, eax
xor	ebx, ebx
lea	rsi, .LC1[rip]
lea	rbp, .LC2[rip]
call	__printf_chk@PLT
.L16:
mov	rdx, rbx
mov	rsi, rbp
mov	edi, 1
xor	eax, eax
call	__printf_chk@PLT
add	rbx, 1
jmp	.L16