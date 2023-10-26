// Copyright (c) 2023 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include <unordered_map>
#include <Python.h>


// Python utility classes & functions

class PyStringHolder
{
public:
	bool operator==(const PyStringHolder& other) const
	{
		return PyUnicode_Compare(str_, other.str_) == 0;
	}
	PyObject* string() { return str_; }

private:
	PyObject* str_;
};


class PyStringHash 
{
	std::size_t operator()(const PyStringHolder& holder) const 
	{
		return PyUnicode_HASH(holder.string());
	}
};

template<typename T>
class PyStringMap : public std::unordered_map<PyStringHolder, T>
{
};



