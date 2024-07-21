// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include "TElement.h"
#include "feature/FeaturePtr.h"
#include "feature/NodePtr.h"
#include "feature/WayPtr.h"
#include "feature/RelationPtr.h"

class Layout;
class TTagTable;
class TRelationTable;
class TileModel;

class TFeature : public TReferencedElement
{
public:
	TFeature(Handle handle, uint32_t size, FeaturePtr feature, int anchor) :
		TReferencedElement(Type::FEATURE, handle, size, Alignment::DWORD, anchor),
		feature_(feature),
		nextById_(nullptr)
	{
	}

	FeaturePtr feature() const { return feature_; }
	uint64_t id() const { return feature_.id(); }
	uint64_t typeCode() const { return feature_.typeCode(); }
	uint64_t idBits() const { return feature_.idBits(); }
	int flags() const { return feature_.flags(); }
	bool isRelationMember() const { return feature_.flags() & FeatureFlags::RELATION_MEMBER; }
	TTagTable* tags(TileModel& tile) const;
	TRelationTable* parentRelations(TileModel& tile) const;
	TFeature* nextFeature() const
	{
		assert(next_ == nullptr || next_->type() == Type::FEATURE);
		return reinterpret_cast<TFeature*>(next_);
	}
	void write(const TileModel& tile) const;

	static void addRelationTable(Layout& layout, DataPtr ppRelTable);

	static bool compareById(const TFeature* a, const TFeature* b)
	{
		return a->id() < b->id();
	}

protected:
	union
	{
		FeaturePtr feature_;
		NodePtr node_;
		WayPtr way_;
		RelationPtr relation_;
	};
	TFeature* nextById_;

	friend class FeatureTable;
};

class TNode : public TFeature
{
public:
	TNode(Handle handle, NodePtr node) :
		TFeature(handle, 20 + (node.flags() & 4),  node, 8)		// Bit 2 = member flag
	{
	}

	void placeBody(Layout& layout)
	{
		if (node_.isRelationMember()) addRelationTable(layout, node_.ptr() + 12);
	}

	Coordinate xy() const noexcept { return Coordinate(x_, y_); }

private:
	int32_t x_;
	int32_t y_;
};

class TWayBody : public TElement
{
public:
	TWayBody(DataPtr data, uint32_t size, uint32_t anchor) :
		TElement(Type::WAY_BODY, 0, size, anchor ? Alignment::WORD : Alignment::BYTE, anchor),
		data_(data)
	{
	}

	DataPtr data() const { return data_; }
	void write(const TileModel& tile) const;

private:
	DataPtr data_;
};

class TWay : public TFeature
{
public:
	TWay(Handle handle, WayPtr way, DataPtr pBodyData, uint32_t bodySize, uint32_t bodyAnchor) :
		TFeature(handle, 32, way, 16),
		body_(pBodyData, bodySize, bodyAnchor)
	{
	}

	const TWayBody& body() const { return body_; }
	void placeBody(Layout& layout);
	
private:
	TWayBody body_;
};


class TRelationBody : public TElement
{
public:
	TRelationBody(DataPtr data, uint32_t size, int anchor) :
		TElement(Type::RELATION_BODY, 0, size, Alignment::WORD, anchor),
		data_(data)
	{
	}

	DataPtr data() const { return data_; }
	void write(const TileModel& tile) const;

private:
	DataPtr data_;
};

class TRelation : public TFeature
{
public:
	TRelation(Handle handle, RelationPtr relation, DataPtr pBodyData, uint32_t bodySize) :
		TFeature(handle, 32, relation, 16),
		body_(pBodyData, bodySize, relation.flags() & 4)		// 4 == member flag
	{
	}

	const TRelationBody& body() const { return body_; }
	void placeBody(Layout& layout);

private:
	TRelationBody body_;
};

// TODO: Use nextById_ for lookup; next_ is used for forming chains or features
class FeatureTable : public Lookup<FeatureTable, TFeature>
{
public:
	static uint64_t getId(TFeature* element)
	{
		return element->feature().idBits();
	}

	static TFeature** next(TFeature* elem)
	{
		return reinterpret_cast<TFeature**>(&elem->nextById_);
	}
};
