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
	TTagTable* tags = tile_.beginTagTable(size, 0);
	MutableDataPtr data = tags->mutableData();
	MutableDataPtr pTag = data;
	MutableDataPtr pPrevKey = pTag;

	TTagTable::Hasher hasher;

	if (taggedSize & 1) // has local keys
	{
		uint32_t localTagsSize = readVarint32(p_) << 1;
		tags->setAnchor(localTagsSize);
		pTag += localTagsSize;
		do
		{
			pTag -= 4;
			uint32_t keyBits = readVarint32(p_);
			TString* keyString = strings_[keyBits >> 2];
			keyString->setAlignment(TElement::Alignment::DWORD);
			hasher.addKey(keyString);
			pPrevKey = pTag;

			// TODO: encode key pointer

			pTag -= (keyBits & 2) + 2;
			encodeTagValue(hasher, pTag, keyBits);
		} 
		while (pTag > data);
		pPrevKey.putUnsignedShort(pPrevKey.getUnsignedShort() | 4);
		pTag += localTagsSize;
	}

	MutableDataPtr end = data + size;
	do
	{
		uint32_t keyBits = readVarint32(p_);
		pPrevKey = pTag;
		pTag.putUnsignedShort(static_cast<uint16_t>(keyBits));
		hasher.addKey(keyBits >> 2);
		pTag += 2;
		encodeTagValue(hasher, pTag, keyBits);
		pTag += (keyBits & 2) + 2;
	} 
	while (pTag < end);
	pPrevKey.putUnsignedShort(pPrevKey.getUnsignedShort() | 0x8000);

	return tile_.completeTagTable(tags, hasher.hash());
}


void TesReader::encodeTagValue(TTagTable::Hasher& hasher, MutableDataPtr p, uint32_t keyBits)
{
	uint32_t value = readVarint32(p_);

	if (keyBits & 2)  // wide value ?
	{
		if (keyBits & 1)  // wide string ?
		{
			TString* valueString = strings_[value];
			hasher.addValue(valueString);
			// TODO: encode pointer
		}
		else
		{
			hasher.addValue(value);
			p.putUnsignedIntUnaligned(value);
		}
	}
	else
	{
		hasher.addValue(value);
		p.putUnsignedShort(value);
	}
}
