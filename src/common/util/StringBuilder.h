// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include <Python.h>
#include "BufferWriter.h"

class StringBuilder : public BufferWriter
{
public:
	StringBuilder(int initialCapacity = 512) : 
		buf_(initialCapacity) 
	{
		setBuffer(&buf_);
	}

	std::string toString()
	{
		flush();
		return std::string(buf_.data(), buf_.length());
	}

	PyObject* toPythonString() 
	{
		flush();
		return PyUnicode_FromStringAndSize(buf_.data(), buf_.length());
	}


private:
	DynamicBuffer buf_;		// TODO: Should use a StackBuffer
};
