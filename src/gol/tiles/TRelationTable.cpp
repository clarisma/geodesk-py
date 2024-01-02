// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#include "TRelationTable.h"
#include "TTile.h"

void TRelationTable::write(const TTile& tile) const
{
	uint8_t* p = tile.newTileData() + location();
	TSharedElement::write(p);

	// TODO: adjust local-feature pointers
}
