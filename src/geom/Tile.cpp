// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#include "Tile.h"
#include <common/util/BufferWriter.h>


char* Tile::formatReverse(char* end) const
{
	end = Format::unsignedIntegerReverse(static_cast<unsigned int>(row()), end) - 1;
	*end = '/';
	end = Format::unsignedIntegerReverse(static_cast<unsigned int>(column()), end) - 1;
	*end = '/';
	return Format::unsignedIntegerReverse(static_cast<unsigned int>(zoom()), end);
}


void Tile::write(BufferWriter& out) const
{
	char buf[32];
	char* end = buf + sizeof(buf);
	char* start = formatReverse(end);
	out.writeBytes(start, end - start);
}
