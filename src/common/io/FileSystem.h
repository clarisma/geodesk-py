// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include <cstring>
#include <stdint.h>

class FileSystem
{
public:
	static size_t getBlockSize(const char* path);
};
