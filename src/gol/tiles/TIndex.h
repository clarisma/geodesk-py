#pragma once

#include "TElement.h"
#include <common/util/pointer.h>
#include "geom/Box.h"

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
	TIndexBranch* firstChildBranch()
	{
		assert(!isLeaf());
		return reinterpret_cast<TIndexBranch*>(firstChild_ & ~1);
	}

	TIndexBranch* next() const { return next_; }
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
	TIndexTrunk(const Box& bounds, TIndexBranch* firstBranch) :
		TIndexBranch(bounds, reinterpret_cast<uintptr_t>(firstBranch) | 1,
			calculateSize(firstBranch))
	{
	}

	void write(uint8_t* p) const;

private:
	static uint32_t calculateSize(TIndexBranch* firstBranch);
};


class TIndex : public TElement
{
public:
	void addFeature(TFeature* feature);

private:
	static const int MAX_CATEGORIES = 30;

	struct Root
	{
		int32_t indexBits;
		uint32_t count;
		TIndexTrunk* trunk;
		TFeature* firstFeature;
	};

	Root roots_[MAX_CATEGORIES + 2];
};


class Indexer
{
private:
	TIndex indexes_[4];			// for nodes, ways, areas & relations
};
