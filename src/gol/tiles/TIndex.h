// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include "TElement.h"
#include <common/util/pointer.h>
#include "geom/Box.h"

class FeatureTable;
class HilbertIndexBuilder;
class IndexSettings;
class Layout;
class TFeature;
class TTile;

class TIndexBranch : public TElement
{
public:
	TIndexBranch(Type type, const Box& bounds, uint32_t size) :
		TElement(type, 0, size, TElement::Alignment::DWORD),
		bounds_(bounds),
		nextSibling_(nullptr)
	{
	}

	bool isLeaf() const { return type() == Type::LEAF; }
	const Box& bounds() const { return bounds_; }
	TIndexBranch* nextSibling() const { return nextSibling_; }
	void setNextSibling(TIndexBranch* next) { nextSibling_ = next; }

private:
	const Box bounds_;
	TIndexBranch* nextSibling_;
};


class TIndexLeaf : public TIndexBranch
{
public:
	TIndexLeaf(const Box& bounds, TFeature* firstFeature) :
		TIndexBranch(Type::LEAF, bounds, calculateSize(firstFeature)),
		firstFeature_(firstFeature)
	{
	}

	void place(Layout& layout);

	TFeature* firstFeature() const { return firstFeature_; }

private:
	static uint32_t calculateSize(TFeature* firstFeature);

	TFeature* firstFeature_;
};

class TIndexTrunk : public TIndexBranch
{
public:
	TIndexTrunk(const Box& bounds, TIndexBranch* firstBranch, int count) :
		TIndexBranch(Type::TRUNK, bounds, count * 20),
		firstBranch_(firstBranch)
	{
	}

	TIndexBranch* firstChildBranch() const { return firstBranch_; }

	void place(Layout& layout);
	void write(const TTile& tile) const;

private:
	TIndexBranch* firstBranch_;
};


class TIndex : public TElement
{
public:
	TIndex();
	void addFeature(TFeature* feature, int category, uint32_t indexBits)
	{
		roots_[category].addFeature(feature, indexBits);
	}

	void build(TTile& tile, const IndexSettings& settings);
	void place(Layout& layout);
	void write(const TTile& tile) const;

	static const int MAX_CATEGORIES = 30;
	static const int NUMBER_OF_ROOTS = MAX_CATEGORIES + 2;
	// includes no-category (first) and multi-category (last)
	static const int MULTI_CATEGORY = NUMBER_OF_ROOTS - 1;
	static const int UNASSIGNED_CATEGORY = 255;

private:

	struct Root
	{
		int32_t indexBits;
		uint32_t featureCount;
		union
		{
			TIndexTrunk* trunk;
			TFeature* firstFeature;
		};

		bool isEmpty() const { return featureCount == 0; }
		void addFeature(TFeature* feature, uint32_t indexBits);
		void add(Root& other);
		void build(HilbertIndexBuilder& rtreeBuilder);
	};

	int getFeatureCategory(TFeature* feature);

	Root roots_[NUMBER_OF_ROOTS];
	int8_t next_[NUMBER_OF_ROOTS];
	int8_t firstRoot_;
	int rootCount_;
};


class Indexer
{
public:
	Indexer(TTile& tile, const IndexSettings& settings);
	void addFeatures(const FeatureTable& features);
	void build();
	void place(Layout& layout);

private:
	static const uint8_t FLAGS_TO_TYPE[16];

	enum
	{
		NODES,
		WAYS,
		AREAS,
		RELATIONS,
		INVALID
	};

	TTile& tile_;
	const IndexSettings& settings_;
	TIndex indexes_[4];			// for nodes, ways, areas & relations
};
