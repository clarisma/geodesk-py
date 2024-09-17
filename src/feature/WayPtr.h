// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include <common/util/varint.h>
#include "feature/NodePtr.h"

class WayPtr : public FeaturePtr
{
public:
	explicit WayPtr(FeaturePtr f) : FeaturePtr(f.ptr()) { assert(isNull() || isWay()); }
	explicit WayPtr(DataPtr p) : FeaturePtr(p) {}

	// TODO: remove in v2
	bool isPlaceholder() const { return maxY() < minY(); }
	uint32_t nodeCount() const noexcept
	{
		const uint8_t* p = bodyptr();
		return readVarint32(p);
	}
};



