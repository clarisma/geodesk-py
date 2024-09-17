// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include "feature/FeaturePtr.h"

class NodePtr : public FeaturePtr
{
public:
	explicit NodePtr(FeaturePtr f) : FeaturePtr(f.ptr()) { assert(isNull() || isNode()); }
	explicit NodePtr(DataPtr p) : FeaturePtr(p) {}
	// explicit NodePtr(const uint8_t* p) : FeaturePtr(p) {}

	int32_t x() const { return (p_-8).getInt(); }
	int32_t y() const { return (p_-4).getInt(); }
	Coordinate xy() const { return Coordinate(x(), y()); }
	Box bounds() const
	{
		return Box(x(), y(), x(), y());
	}

	// TODO: remove in v2
	bool isPlaceholder() const { return (p_-8).getLongUnaligned() == 0; }

	bool intersects(const Box& bounds) const
	{
		return bounds.contains(x(), y());
	}

	/**
	 * Obtains a pointer to the node's relation table.
	 * The node must belong to a relation, otherwise the
	 * result is undefined.
	 */
	DataPtr relationTableFast() const
	{
		return (p_ + 12).follow();
	}
};


struct NodeStruct
{
	Coordinate xy;
	FeatureHeader header;
	int32_t tagTablePtr;		// tagged
	int32_t relTablePtr;		// can be omitted
};
