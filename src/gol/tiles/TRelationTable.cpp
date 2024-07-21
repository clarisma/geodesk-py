// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#include "TRelationTable.h"
#include "TileKit.h"

void TRelationTable::write(const TileKit& tile) const
{
	uint8_t* p = tile.newTileData() + location();
	TSharedElement::write(p);

	// TODO: adjust local-feature pointers
}
