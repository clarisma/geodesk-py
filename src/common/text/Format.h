// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once
#include <cstdint>
#include <iostream>
#include <sstream>
#include <thread>
#include <string>

namespace Format
{
	inline std::string format(std::thread::id id)
	{
        std::stringstream ss;
        ss << id;
        return ss.str();
	}
}