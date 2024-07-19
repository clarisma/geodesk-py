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
	class SortedFeature
	{
	public:
		SortedFeature(TFeature* feature) :
			typeAndId_(feature->id() |
				(static_cast<uint64_t>(feature->typeCode()) << 60)),
			feature_(feature)
		{
		}

		uint64_t id() const { return typeAndId_ & 0xff'ffff'ffff'ffffULL; }
		int typeCode() const { return static_cast<int>(typeAndId_ >> 60); }
		TFeature* feature() const { return feature_; }

		bool operator<(const SortedFeature& other) const
		{
			return typeAndId_ < other.typeAndId_;
		}

	private:
		uint64_t typeAndId_;
		TFeature* feature_;
	};

	struct Tag
	{
		uint32_t key;
		uint32_t value;

		Tag(uint32_t k, uint32_t v) : key(k), value(v) {}
	};

	template <typename T>
	void gatherSharedItems(const ElementDeduplicator<T>& items, int minUsers, size_t firstGroupSize);

	void writeFeatureIndex();

	void writeStrings();
	void writeTagTables();
	void writeRelationTables();
	void writeTagTable(const TTagTable* tags);
	uint32_t getTagValue(pointer p, int valueFlags);
	void writeRelationTable(const TRelationTable* relTable);
	void writeFeatures();
	void writeNode(const TNode* node);
	void writeWay(const TWay* way);
	void writeRelation(const TRelation* relation);
	void writeStub(const TFeature* feature, int flags);
	void writeBounds(FeatureRef feature);

	BufferWriter out_;
	TTile& tile_;
	Coordinate prevXY_;
	std::vector<SortedFeature> features_;
	int nodeCount_;
	int wayCount_;
	std::vector<TSharedElement*> sharedElements_;
	std::vector<Tag> localKeyTags_;
};
