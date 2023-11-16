#pragma once

#include "TElement.h"
#include <common/util/pointer.h>
#include "feature/Node.h"
#include "feature/Way.h"
#include "feature/Relation.h"

class TFeature : public TIndexedElement
{
public:
	TFeature(int32_t loc, uint32_t size, FeatureRef feature) :
		TIndexedElement(loc, size, Alignment::DWORD),
		feature_(feature),
		nextById_(nullptr)
	{
	}

	FeatureRef feature() const { return feature_; }
	uint64_t idBits() const { return feature_.idBits(); }
	int flags() const {	return feature_.flags(); }

	TFeature* next() const { return nextById_; }
	void setNext(TFeature* next) { nextById_ = next; }

	void write(const TTile* tile, uint8_t* p) const;

private:
	union
	{
		FeatureRef feature_;
		NodeRef node_;
		WayRef way_;
		RelationRef relation_;
	};
	TFeature* nextById_;

	friend class FeatureTable;
};

class TNode : public TFeature
{
public:
	TNode(int32_t loc, NodeRef node) :
		TFeature(loc, 20 + (node.flags() & 4), node)		// Bit 2 = member flag
	{
	}
};

class TWayBody : public TElement
{
public:
	TWayBody(pointer data, uint32_t size, uint32_t anchor) :
		TElement(-1, size, anchor ? Alignment::WORD : Alignment::BYTE),
		data_(data),
		anchor_(anchor)
	{
	}

private:
	pointer data_;
	uint32_t anchor_;
};

class TWay : public TFeature
{
public:
	TWay(int32_t loc, WayRef way, pointer pBodyData, uint32_t bodySize, uint32_t bodyAnchor) :
		TFeature(loc, 32, way),
		body_(pBodyData, bodySize, bodyAnchor)
	{
	}

private:
	TWayBody body_;
};


class TRelationBody : public TElement
{
public:
	TRelationBody(pointer data, uint32_t size) :
		TElement(-1, size, Alignment::WORD),
		data_(data)
	{
	}

private:
	pointer data_;
};

class TRelation : public TFeature
{
public:
	TRelation(int32_t loc, RelationRef relation, pointer pBodyData, uint32_t bodySize) :
		TFeature(loc, 32, relation),
		body_(pBodyData, bodySize)
	{
	}

private:
	TRelationBody body_;
};


class FeatureTable : public Lookup<FeatureTable, TFeature>
{
public:
	

protected:
	static int64_t getId(TFeature* element)
	{
		return element->location();
	}

	static TFeature** next(TFeature* elem)
	{
		return &elem->nextById_;
	}

	friend class Lookup<FeatureTable, TFeature>;
};
