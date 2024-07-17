// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include <common/util/BufferWriter.h>
#include "geom/Box.h"
#include "geom/Coordinate.h"
#include "TTile.h"

class TNode;
class TWay;
class TRelation;

class TesWriter
{
public:
	TesWriter(TTile& tile, Buffer* out);

	void write();

private:
	template <typename T>
	T** sortedItems(const ElementDeduplicator<T>& items);

	void writeFeatureIndex();

	void writeStrings();
	void writeTagTables();
	void writeRelationTables();
	void writeTagTable(const TTagTable* tags);
	void writeTagValue(pointer p, int valueFlags);
	void writeRelationTable(const TRelationTable* relTable);
	void writeNode(const TNode* node);
	void writeWay(const TWay* way);
	void writeRelation(const TRelation* relation);
	void writeStub(const TFeature* feature, int flags);
	void writeBounds(FeatureRef feature);
	void writeFeatures();

	BufferWriter out_;
	TTile& tile_;
	Coordinate prevXY_;
};
