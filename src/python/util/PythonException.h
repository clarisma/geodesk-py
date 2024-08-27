// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include <stdexcept>

class PythonException : public std::runtime_error
{
public:
	explicit PythonException()
		: std::runtime_error("A Python exception occurred")
	{
	}
};