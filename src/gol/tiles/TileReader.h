// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once
#include "TileReaderBase.h"
#include <common/util/DataPtr.h>
#include <common/util/TaggedPtr.h>
#include <feature/NodePtr.h>
#include <feature/WayPtr.h>
#include <feature/RelationPtr.h>
#include "TileKit.h"

class TString;
class TTagTable;
class TRelationTable;


class TileReader : public TileReaderBase<TileReader>
{
public:
	TileReader(TileKit& tile) : tile_(tile) {}

	void readTile(const DataPtr pTile);

private:
	void readNode(NodePtr node);
	void readWay(WayPtr way);
	void readRelation(RelationPtr relation);
	TString* readString(DataPtr p);
	TTagTable* readTagTable(TagTablePtr pTags);
	TTagTable* readTagTable(FeaturePtr feature);
	TRelationTable* readRelationTable(DataPtr p);

	TileKit& tile_;
};
