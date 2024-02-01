// Copyright (c) 2023 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once
#include <string_view>
#include "build/util/BuildSettings.h"

class GolBuilder
{
public:
	GolBuilder();

	#ifdef GEODESK_PYTHON
	static PyObject* build(PyObject* args, PyObject* kwds);
	void setOptions(PyObject* dict)
	{
		settings_.setOptions(dict);
	}
	#endif

	void build(const char* golPath);

private:
	BuildSettings settings_;
};
