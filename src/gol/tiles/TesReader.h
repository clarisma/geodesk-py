// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include "TileKit.h"
#include <common/util/MutableDataPtr.h>
#include <common/util/TaggedPtr.h>


class TesReader
{
public:
	TesReader(TileKit& tile);

private:
	void readFeatureIndex();
	TString* readString();
	void readStrings();
	TTagTable* readTagTable();
	void encodeTagValue(TTagTable::Hasher& hasher, MutableDataPtr p, uint32_t keyBits);

	TileKit& tile_;
	const uint8_t* p_;
	TString** strings_;
	TTagTable** tagTables_;
	TRelationTable** relationTables_;
	TaggedPtr<TFeature,1>* features_;
};
