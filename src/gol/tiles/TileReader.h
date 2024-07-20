// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once
#include "TileReaderBase.h"
#include <common/util/DataPtr.h>
#include <common/util/TaggedPtr.h>
#include "TileKit.h"

class TileReader : public TileReaderBase<TileReader>
{
public:
	TileReader(TileKit& tile) : tile_(tile) {}

	void readTile(const DataPtr pTile);

private:
	void readNode(NodeRef node);
	void readWay(WayRef way);
	void readRelation(RelationRef relation);
	TString* readString(DataPtr p);
	TTagTable* readTagTable(TaggedPtr<const uint8_t, 1> pTagged);
	TTagTable* readTagTable(FeatureRef feature)
	{
		TTagTable* tags = readTagTable(feature.tags().taggedPtr());
		tags->addUser();
		return tags;
	}
	TRelationTable* readRelationTable(pointer p);


	TileKit& tile_;
};
