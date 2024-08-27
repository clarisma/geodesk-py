// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once
#include <cstdint>
#include <iostream>
#include <sstream>
#include <thread>
#include <string>

namespace Threads
{
	inline std::string currentThreadId()
	{
		std::stringstream ss;
		ss << std::this_thread::get_id();
		return ss.str();
	}
}