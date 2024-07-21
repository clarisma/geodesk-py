// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#include "TTagTable.h"
#include "TileModel.h"
#include "IndexSettings.h"

void TTagTable::write(const TileModel& tile) const
{
	uint8_t* p = tile.newTileData() + location();
	TSharedElement::write(p);

	// TODO: adjust string pointers
}


uint32_t TTagTable::assignIndexCategory(const IndexSettings& indexSettings)
{
	int maxIndexedKey = indexSettings.maxIndexedKey();
	int category = 0;
	uint32_t indexBits = 0;
	pointer p = data_ + anchor();
	int keyWithLastFlag;
	do
	{
		uint16_t keyBits = p.getUnsignedShort();
		keyWithLastFlag = keyBits >> 2;
		int keyCategory = indexSettings.getCategory(keyWithLastFlag & 0x1FFF);
		if (keyCategory > 0)
		{
			if (category != 0)
			{
				category = TIndex::MULTI_CATEGORY;
			}
			else
			{
				category = keyCategory;
			}
			assert (keyCategory >= 1 && keyCategory <= TIndex::MAX_CATEGORIES);
			indexBits |= (1 << (keyCategory - 1));
		}
		p += 2 + (keyBits % 2);
	}
	while (keyWithLastFlag < maxIndexedKey);
	category_ = category;
	return indexBits;
}


void TTagTable::addStrings(Layout& layout) const
{
	// TODO
}


