// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include <common/util/StringBuilder.h>

class Highlighter
{
public:
	static void highlight(StringBuilder& buf, const char* text, 
		int ofs, int len, int color);

private:
	static const int MAX_LINE_LEN = 74;
};
