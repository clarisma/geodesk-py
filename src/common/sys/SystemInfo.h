// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once
#include <cstdint>

class SystemInfo
{
public:
	SystemInfo();

	size_t minWorkingSet;
	size_t maxWorkingSet;
};
