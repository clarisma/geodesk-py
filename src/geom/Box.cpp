// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#include "Box.h"
#include <common/text/Format.h>

void Box::format(char* buf) const
{
	Format::unsafe(buf, "[(%d,%d),(%d,%d)]", minX(), minY(), maxX(), maxY());
}

std::string Box::toString() const
{
	char buf[64];
	format(buf);
	return std::string(buf);
}
