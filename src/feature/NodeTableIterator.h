// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include <common/util/DataPtr.h>
#include "feature/NodePtr.h"
#include "feature/Tex.h"
#include "feature/types.h"


class NodeTableIterator
{
public:
	NodeTableIterator(int_fast32_t handle, DataPtr pTable) :
		pTile_(pTable - handle),
		ofs_(handle),
		currentOfs_(0),
		node_(0),
		tipDelta_(0)
	{
	}

	bool next()
	{
		if (node_ & MemberFlags::LAST) return false;
#ifdef GEODESK_READ_V2
		rel_ = p_.getUnsignedShort();
		// TODO
#else
		ofs_ -= 4;
		currentOfs_ = ofs_;
		node_ = (pTile_ + ofs_).getIntUnaligned();
#endif
		if ((node_ & (MemberFlags::FOREIGN | MemberFlags::DIFFERENT_TILE)) ==
			(MemberFlags::FOREIGN | MemberFlags::DIFFERENT_TILE))
		{
			// foreign node in different tile
			ofs_ -= 2;
			tipDelta_ = (pTile_ + ofs_).getShort();
			if (tipDelta_ & 1)
			{
				// wide TIP delta
				ofs_ -= 2;
				tipDelta_ = (tipDelta_ & 0xffff) |
					(static_cast<int32_t>((pTile_ + ofs_).getShort()) << 16);
			}
			tipDelta_ >>= 1;     // signed
		}
		return true;
	}

	bool isForeign() const { return node_ & MemberFlags::FOREIGN; }
	bool isInDifferentTile() const
	{ 
		assert(isForeign());
		return node_ & MemberFlags::DIFFERENT_TILE;
	}
	
	TipDelta tipDelta() const { return TipDelta(tipDelta_); }
	// DataPtr currentLocal() const { return pCurrent_; }
	int_fast32_t localNodeHandle() const
	{
		return currentOfs_ + (static_cast<int_fast32_t>(node_ & 0xffff'fffc) >> 1);
		// TODO: shift should not be needed, flaw in Tile Spec 1.0
		// no, shift is needed, but mask is not (Bit 1 is always 0 and moves to Bit 0,
		// so pointer is 2-byte aligned which is sufficient)
		// Should be: return currentOfs_ + (node_ >> 1);
	}
	NodePtr localNode() const
	{
		return NodePtr(pTile_ + localNodeHandle());
	}
	TexDelta texDelta() const
	{
		return node_ >> 4;
	}

protected:
	DataPtr pTile_;
	int_fast32_t ofs_;
	int_fast32_t currentOfs_;
	int32_t node_;
	int32_t tipDelta_;
};

