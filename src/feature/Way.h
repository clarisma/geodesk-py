/*
// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include "Node.h"

class WayRef : public FeatureRef
{
public:
	explicit WayRef(FeatureRef f) : FeatureRef(f.ptr()) { assert(isNull() || isWay()); }
	explicit WayRef(pointer p) : FeatureRef(p) {}

	bool isPlaceholder() const { return maxY() < minY(); }
	uint32_t nodeCount() const;

	bool containsPointFast(double cx, double cy);
};

// TODO: could store the flags in this struct, there is room anyway due to padding
class WayCoordinateIterator
{
public:
	WayCoordinateIterator() {};
	WayCoordinateIterator(WayRef way);

	// TODO: Fix the flags vs. bool issue, because it could lead to unintended behavior:
	// flags can be silently passed as duplicateFirst, so even if a feature is not an
	// area, it will be treated as such since area-flag is not isolated
	// Better to make FeatureFlags a type-safe enum class!
	void start(const uint8_t* p, int32_t prevX, int32_t prevY, bool duplicateFirst);
	void start(const FeatureRef way, int flags);
	Coordinate next();

	// does not include any duplicated last coordinate 
	int storedCoordinatesRemaining() const { return remaining_; }
	// This one includes any duplicate last coordinate for areas, based on flags:
	int coordinatesRemaining() const
	{
		return remaining_ + (duplicateFirst_ ? 1 : 0);
	}

private:
	const uint8_t* p_;
	int remaining_;
	bool duplicateFirst_;
	int32_t x_;				// TODO: use Coordinate
	int32_t y_;
	int32_t firstX_;
	int32_t firstY_;
};

class FeatureNodeIterator
{
public:
	FeatureNodeIterator(FeatureStore* store);
	FeatureStore* store() const { return store_; }
	void start(pointer pBody, int flags, const MatcherHolder* matcher, const Filter* filter); 
	NodeRef next();

private:
	FeatureStore* store_;
	const MatcherHolder* matcher_;
	const Filter* filter_;
	Tip currentTip_;
	int32_t currentNode_;
	pointer p_;
	pointer pForeignTile_;
};
*/

