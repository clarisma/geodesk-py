// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include "Feature.h"

class NodeRef : public FeatureRef
{
public:
	explicit NodeRef(FeatureRef f) : FeatureRef(f.ptr()) { assert(isNull() || isNode()); }
	explicit NodeRef(pointer p) : FeatureRef(p) {}

	int32_t x() const { return ptr_.getInt(-8); }
	int32_t y() const { return ptr_.getInt(-4); }
	Coordinate xy() const { return Coordinate(x(), y()); }
	Box bounds() const
	{
		return Box(x(), y(), x(), y());
	}
	bool isPlaceholder() const { return ptr_.getUnalignedLong(-8) == 0; }
	bool intersects(const Box& bounds) const
	{
		return bounds.contains(ptr_.getInt(-8), ptr_.getInt(-4));
	}
};