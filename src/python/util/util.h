// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include <Python.h>
#include <string_view>

namespace Python
{
	extern PyObject* createSet(const char** strings);
	extern PyObject* createList(const char** strings, size_t count);
	extern PyObject* formatString(PyObject* templateString, PyObject* object);
	extern std::string_view stringAsStringView(PyObject* str);
	extern std::string_view objectAsStringView(PyObject* str);

	extern void createDirMethod(PyTypeObject* type, PyCFunctionWithKeywords dirFunc);

	/**
	 * Checks if args/kwargs contain a single argument.
	 * If so, returns a *borrowed* reference to this argument.
	 * Otherwise, raises a TypeError and returns NULL.
	 * 
	 * @param what a free-form string of what argument types are expected
	 */
	extern PyObject* checkSingleArg(PyObject* args, PyObject* kwargs, const char* what);
	
	/**
	 * Checks if args/kwargs contain a single argument of the given type.
	 * If so, returns a *borrowed* reference to this argument.
	 * Otherwise, raises a TypeError and returns NULL.
	 */
	extern PyObject* checkSingleArg(PyObject* args, PyObject* kwargs, PyTypeObject* type);

	extern PyObject* checkType(PyObject* arg, PyTypeObject* type);
	extern PyObject* checkType(PyObject* arg, PyTypeObject* type, const char* what);
	extern PyObject* checkNumeric(PyObject* arg);

	inline PyObject* badKeyword(const char* str)
	{
		PyErr_Format(PyExc_TypeError, "%s: invalid keyword argument", str);
		return NULL;
	}

	inline PyObject* badArgumentType(PyTypeObject* type)
	{
		PyErr_Format(PyExc_TypeError, "Invalid argument type (%s)", type->tp_name);
		return NULL;
	}

	inline bool isNumeric(PyObject* obj)
	{
		return PyFloat_Check(obj) || PyLong_Check(obj);
	}

	/**
	 * Checks if the given object is iterable.
	 */
	inline bool isIterable(PyObject* obj)
	{
		return obj->ob_type->tp_iter != NULL || PySequence_Check(obj);
	}

	template <typename T>
	T* alloc(PyTypeObject* type)
	{
		return (T*)type->tp_alloc(type, 0);
	}

	template <typename T>
	T* newRef(T* obj)
	{
		// (Py_NewRef exists only since 3.10)
		// return (T*)Py_NewRef((PyObject*)obj);
		Py_INCREF((PyObject*)obj);
		return (T*)obj;
	}

	inline PyObject* boolValue(bool b)
	{
		return newRef(b ? Py_True : Py_False);
	}

	inline void set(PyObject** ptr, PyObject* newVal)
	{
		PyObject* tmp = *ptr;
		Py_XINCREF(newVal);
		*ptr = newVal;
		Py_XDECREF(tmp);
	}

	inline PyObject* getStringOrDefault(PyObject* obj, const char* s)
	{
		if (obj) return newRef(obj);
		return PyUnicode_FromString(s);
	}

	inline PyObject* getObjectOrNone(PyObject* obj)
	{
		return newRef(obj ? obj : Py_None);
	}

	inline int setBool(PyObject* obj, bool* pv)
	{
		if (!Python::checkType(obj, &PyBool_Type)) return -1;
		*pv = PyObject_IsTrue(obj);
		return 0;
	}

	inline int setLong(PyObject* obj, int64_t* pv, int64_t min, int64_t max)
	{
		if (!Python::checkNumeric(obj)) return -1;
		int64_t value = PyLong_AsLong(obj);
		if (value < min)
		{
			PyErr_Format(PyExc_ValueError, "Must be at least %d", min);
			return -1;
		}
		if (value > max)
		{
			PyErr_Format(PyExc_ValueError, "Must not exceed %d", max);
			return -1;
		}
		*pv = value;
		return 0;
	}

	inline int setInt(PyObject* obj, int* pv, int min, int max)
	{
		int64_t value;
		if (setLong(obj, &value, min, max) < 0) return -1;
		*pv = (int)value;
		return 0;
	}

	inline PyObject* callOneArg(PyObject* callable, PyObject* arg)
	{
		#if PY_VERSION_HEX >= 0x03090000
		// Function introduced in Python 3.9
		return PyObject_CallOneArg(callable, arg);
		#else
		// Code for earlier versions
		PyObject* args = PyTuple_Pack(1, arg);
		PyObject* result = PyObject_CallObject(callable, args);
		Py_DECREF(args);
		return result;
		#endif
	}

	typedef PyObject* (*Getter)(PyObject*);

	class AttrRef
	{
	private:
		uint64_t taggedPtr_;
		
	public:
		AttrRef() {} // TODO: appeased compiler
		constexpr AttrRef(uint64_t p) : taggedPtr_(p) {}

		static AttrRef property(Getter getter)
		{
			return AttrRef(reinterpret_cast<uint64_t>(getter) << 1);
		}
		static AttrRef method(PyCFunctionWithKeywords method)
		{
			return AttrRef((reinterpret_cast<uint64_t>(method) << 1) | 1);
		}
		bool isMethod() const { return taggedPtr_ & 1; }
		Getter getter() const 
		{
			assert(!isMethod());
			return reinterpret_cast<Getter>(taggedPtr_ >> 1);
		}
		PyCFunctionWithKeywords method() const
		{
			assert(isMethod());
			return reinterpret_cast<PyCFunctionWithKeywords>(taggedPtr_ >> 1);
		}
	};
}

#define ATTR_PROPERTY(p) (Python::AttrRef::property((Python::Getter)&p))
#define ATTR_METHOD(m) (Python::AttrRef::method((PyCFunctionWithKeywords)&m))
