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

	void writeStrings();
	void writeTagTable(const TTagTable* tags);
	void writeTagValue(pointer p, int valueFlags);
	void writeNode(const TNode* node);
	void writeWay(const TWay* way);
	void writeRelation(const TRelation* relation);
	void writeStub(const TFeature* feature, int flagBitCount, int flags);
	void writeBounds(FeatureRef feature);

	static const int TAGS_CHANGED = 1;
	static const int RELATIONS_CHANGED = 2;
	static const int GEOMETRY_CHANGED = 4;
	static const int AREA_FEATURE = 8;
	static const int WAY_HAS_FEATURE_NODES = 16;

	TString** strings_;
	TTagTable** tagTables_;
	TRelationTable** relationTables_;
	TFeature** features_;
	BufferWriter out_;
	TTile& tile_;
	Coordinate prevXY_;
	uint64_t prevId_;
};
