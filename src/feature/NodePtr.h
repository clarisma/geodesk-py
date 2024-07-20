// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include "FeaturePtr.h"

class NodePtr : public FeaturePtr
{
public:
	explicit NodePtr(FeaturePtr f) : FeaturePtr(f.ptr()) { assert(isNull() || isNode()); }
	explicit NodePtr(DataPtr p) : FeaturePtr(p) {}

	int32_t x() const { return (p_-8).getInt(); }
	int32_t y() const { return (p_-4).getInt(); }
	Coordinate xy() const { return Coordinate(x(), y()); }
	Box bounds() const
	{
		return Box(x(), y(), x(), y());
	}
	bool intersects(const Box& bounds) const
	{
		return bounds.contains(x(), y());
	}
};