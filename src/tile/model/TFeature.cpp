// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#include "TFeature.h"
#include "Layout.h"
#include "TileModel.h"

TTagTable* TFeature::tags(TileModel& tile) const
{
	// TODO: fix so it can be used for new handles as well
	TElement::Handle handle = tile.existingHandle(feature_.tags().ptr());
	return tile.getTags(handle);
}


TRelationTable* TFeature::parentRelations(TileModel& tile) const
{
	if (!feature_.isRelationMember()) return nullptr;
	// TODO: fix so it can be used for new handles as well
	TElement::Handle handle = tile.existingHandle(feature_.relationTableFast());
	return tile.getRelationTable(handle);
}

void TFeature::write(const TileModel& tile) const
{
	uint8_t* p = tile.newTileData() + location();
	int32_t* pInt;
	if (feature_.isNode())
	{
		memcpy(p, feature_.ptr() - 8, 16);
		pInt = reinterpret_cast<int32_t*>(p + 8);
	}
	else
	{
		memcpy(p, feature_.ptr() - 16, 24);
		pInt = reinterpret_cast<int32_t*>(p + 16);
	}
	*pInt = (*pInt & ~1) | (isLast() ? 1 : 0);
		// set the is_last flag bit 

	// TODO: encode tag-table pointer
	// TODO: encode body pointer
}


void TFeature::addRelationTable(Layout& layout, DataPtr ppRelTable)
{
	TileModel& tile = layout.tile();
	
	// TODO: fix, need handle for new elemetns as well
	DataPtr pRelTable = ppRelTable.followUnaligned();
	TRelationTable* relTable = tile.getRelationTable(tile.existingHandle(pRelTable));
	assert(relTable);
	if (relTable->location() <= 0)
	{
		layout.place(relTable);
	}
}

void TWay::placeBody(Layout& layout)
{
	layout.addBodyElement(&body_);
	if (isRelationMember())
	{
		addRelationTable(layout, body_.data() + body_.anchor() - 4);
	}
}


void TRelation::placeBody(Layout& layout)
{
	layout.addBodyElement(&body_);
	if (isRelationMember())
	{
		addRelationTable(layout, body_.data() + body_.anchor() - 4);
	}

	// TODO: place role strings
}


void TWayBody::write(const TileModel& tile) const
{
	uint8_t* p = tile.newTileData() + location();
	memcpy(p, data_, size());

	// TODO: adjust pointers to local nodes
}

void TRelationBody::write(const TileModel& tile) const
{
	uint8_t* p = tile.newTileData() + location();
	memcpy(p, data_, size());

	// TODO: adjust pointers to local featues and role strings
}
