// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include "FeaturePtr.h"
#include "types.h"

// TODO

class AbstractFeatureTableIterator
{
public:
	AbstractFeatureTableIterator(DataPtr pTable) :
		p_(pTable),
		pCurrent_(pTable),
		item_(0),
		tipDelta_(0)
	{
	}

	bool isForeign() const { return item_ & MemberFlags::FOREIGN; }
	bool isInDifferentTile() 
	{ 
		assert(isForeign());
		return item_ & MemberFlags::DIFFERENT_TILE;
	}

	TipDelta tipDelta() const { return TipDelta(tipDelta_); }
	DataPtr currentLocalPtr() const { return pCurrent_; }

private:
	DataPtr p_;
	DataPtr pCurrent_;
	int32_t item_;
	int32_t tipDelta_;
};

