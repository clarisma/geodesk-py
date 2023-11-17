#pragma once

#include "TElement.h"
#include <common/util/pointer.h>
#include "geom/Box.h"

class FeatureTable;
class HilbertIndexBuilder;
class IndexSettings;
class TFeature;
class TTile;

class TIndexBranch : public TElement
{
public:
	TIndexBranch(const Box& bounds, uintptr_t firstChild, uint32_t size) :
		TElement(0, size, TElement::Alignment::DWORD),
		firstChild_(firstChild),
		next_(nullptr),
		bounds_(bounds)
	{
	}

	bool isLeaf() const { return (firstChild_ & 1) == 0; }
	TFeature* firstFeature() const 
	{ 
		assert(isLeaf());
		return reinterpret_cast<TFeature*>(firstChild_); 
	}
	TIndexBranch* firstChildBranch() const
	{
		assert(!isLeaf());
		return reinterpret_cast<TIndexBranch*>(firstChild_ & ~1);
	}

	TIndexBranch* next() const { return next_; }
	void setNext(TIndexBranch* next) { next_ = next; }
	const Box& bounds() const { return bounds_; }

private:
	TIndexBranch* next_;
	uintptr_t firstChild_;  // Tagged pointer
							// Bit 0 == 0: pointer to features
							// Bit 0 == 1: pointer to another branch
	const Box bounds_;
};


class TIndexLeaf : public TIndexBranch
{
public:
	TIndexLeaf(const Box& bounds, TFeature* firstFeature) :
		TIndexBranch(bounds, reinterpret_cast<uintptr_t>(firstFeature), 
			calculateSize(firstFeature))
	{
	}

	void write(const TTile* tile, uint8_t* p) const;

private:
	static uint32_t calculateSize(TFeature* firstFeature);
};

class TIndexTrunk : public TIndexBranch
{
public:
	TIndexTrunk(const Box& bounds, TIndexBranch* firstBranch, int count) :
		TIndexBranch(bounds, reinterpret_cast<uintptr_t>(firstBranch) | 1,
			count * 20)
	{
	}

	void write(uint8_t* p) const;

private:
	// static uint32_t calculateSize(TIndexBranch* firstBranch);
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
	void write(uint8_t* p) const;

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
	int8_t lastRoot_;
	int rootCount_;
};


class Indexer
{
public:
	Indexer(TTile& tile, const IndexSettings& settings);
	void addFeatures(const FeatureTable& features);
	void build();

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
