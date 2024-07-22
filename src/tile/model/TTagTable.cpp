// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#include "TTagTable.h"
#include "Layout.h"
#include "TileModel.h"
#include "tile/compiler/IndexSettings.h"

void TTagTable::write(const TileModel& tile) const
{
	uint8_t* p = tile.newTileData() + location();
	TSharedElement::write(p);

	// TODO: adjust string pointers
}

// TODO: move this to compiler
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


void TTagTable::addString(Layout& layout, DataPtr pStr)
{
	// TODO: handle to new strings
	TileModel& tile = layout.tile();
	TElement::Handle handle = layout.tile().existingHandle(pStr);
	TString* str = tile.getString(handle);
	if (str->location() == 0)
	{
		layout.addBodyElement(str);
	}
}

void TTagTable::addStrings(Layout& layout) const
{
	TagTablePtr pTags = tags();
	LocalTagIterator localTags(pTags);
	while (localTags.next())
	{
		addString(layout, localTags.keyString());
		if (localTags.hasLocalStringValue())
		{
			addString(layout, localTags.stringValueFast());
		}
	}

	GlobalTagIterator globalTags(pTags);
	while (globalTags.next())
	{
		if (globalTags.hasLocalStringValue())
		{
			/*
			printf("- %s\n", globalTags.stringValueFast().cast<const ShortVarString>()
				->toString().c_str());
			*/
			addString(layout, globalTags.stringValueFast());
		}
	}
}


