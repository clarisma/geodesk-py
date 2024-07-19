// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include "TileKit.h"

class TesReader
{
public:
	TesReader();

private:
	void readString();

	TileKit& tile_;
	const uint8_t* p_;
};
