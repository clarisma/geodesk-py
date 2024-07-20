// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#include "TesReader.h"
#include <common/util/varint.h>

TesReader::TesReader()
{

}


void TesReader::readFeatureIndex()
{
	uint32_t featureCount = readVarint32(p_);
	tile_.arena().allocArray<TaggedPtr<TFeature, 1>>(featureCount);
	TaggedPtr<TFeature, 1>* end = features_ + featureCount;
	TaggedPtr<TFeature, 1>* ppFeature = features_;
	int type = 0;
	uint64_t prevId = 0;
	while (ppFeature < end)
	{
		uint64_t ref = readVarint64(p_);
		if (ref == 0)
		{
			type++;
			prevId = 0;
			continue;
		}
		uint64_t id = (ref >> 1) + prevId;
		int changeFlag = ref & 1;
		TFeature* feature = tile_.getFeature(TypedFeatureId::ofTypeAndId(type, id));
		if (!feature)
		{
			// If feature has not been marked as changed and it does not exist,
			// we have a referential integrity problem
			// However, don't report it yet, because it may get resolved
			// (Reapplying updates may cause this)
			// 
			// TODO: Create the TFeature
		}
		*ppFeature++ = TaggedPtr<TFeature,1>(feature, changeFlag);
		prevId = id;
	}
}


void TesReader::readStrings()
{
	uint32_t stringCount = readVarint32(p_);
	strings_ = tile_.arena().allocArray<TString*>(stringCount);
	for (int i = 0; i < stringCount; i++) strings_[i] = readString();
}

TString* TesReader::readString()
{
	uint32_t size = TString::getStringSize(p_);

	// Check if string exists already
	TString* string;

	uint8_t* copy = tile_.alloc(size);
	memcpy(copy, p_, size);
	p_ += size;
	return tile_.addString(tile_.newHandle(), copy, size);
}


TTagTable* TesReader::readTagTable()
{
	uint32_t taggedSize = readVarint32(p_);
	uint32_t size = taggedSize & 0xffff'fffe;
	uint8_t* tags = tile_.alloc(size);
	uint8_t* end = tags + size;
	uint8_t* pTag = tags;
	if (taggedSize & 1) // has local keys
	{
		uint32_t localTagCount = readVarint32(p_);
		for (int i = 0; i < localTagCount; i++)
		{
			uint32_t key = readVarint32(p_);
			TString* keyString = strings_[key >> 2];
			keyString->setAlignment(TElement::Alignment::DWORD);

		}
	}
}
