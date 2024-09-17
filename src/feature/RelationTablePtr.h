// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include <common/util/DataPtr.h>
#include "feature/RelationPtr.h"
#include "feature/Tex.h"
#include "feature/types.h"

class RelationTablePtr
{
public:
	RelationTablePtr(DataPtr p) : p_(p) {}
	DataPtr ptr() const { return p_; }

private:
	DataPtr p_;
};


class RelationTableIterator
{
public:
	RelationTableIterator(int_fast32_t handle, RelationTablePtr rels) :
		pTile_(rels.ptr() - handle),
		ofs_(handle),
		currentOfs_(0),
		rel_(0),
		tipDelta_(0)
	{
	}

	bool next()
	{
		if (rel_ & MemberFlags::LAST) return false;
		currentOfs_ = ofs_;
#ifdef GEODESK_READ_V2
		rel_ = p_.getUnsignedShort();
		// TODO
#else
		rel_ = (pTile_ + ofs_).getIntUnaligned();
		ofs_ += 4;
#endif
		if ((rel_ & (MemberFlags::FOREIGN | MemberFlags::DIFFERENT_TILE)) ==
			(MemberFlags::FOREIGN | MemberFlags::DIFFERENT_TILE))
		{
			// foreign relation in different tile
			tipDelta_ = (pTile_ + ofs_).getShort();
			ofs_ += 2;
			if (tipDelta_ & 1)
			{
				// wide TIP delta
				tipDelta_ = (tipDelta_ & 0xffff) |
					(static_cast<int32_t>((pTile_ + ofs_).getShort()) << 16);
				ofs_ += 2;
			}
			tipDelta_ >>= 1;     // signed
		}
		return true;
	}

	bool isForeign() const { return rel_ & MemberFlags::FOREIGN; }
	bool isInDifferentTile() const
	{ 
		assert(isForeign());
		return rel_ & MemberFlags::DIFFERENT_TILE;
		// TODO: flag changes in 2.0
	}
	
	int_fast32_t localRelationHandle() const
	{
		assert(!isForeign());
		return currentOfs_ + (static_cast<int_fast32_t>(rel_ & 0xffff'fffc) >> 1);
		// TODO: shift should not be needed, flaw in Tile Spec 1.0
		// no, shift is needed, but mask is not (Bit 1 is always 0 and moves to Bit 0,
		// so pointer is 2-byte aligned which is sufficient)
		// Should be: return currentOfs_ + (rel_ >> 1);

	}
	
	RelationPtr localRelation() const
	{
		return RelationPtr(pTile_ + localRelationHandle());
	}
	
	TipDelta tipDelta() const { return TipDelta(tipDelta_); }
	TexDelta texDelta() const
	{
		return rel_ >> 4;		// TODO: un-zigzag
	}

	DataPtr ptr() const { return pTile_ + ofs_; }
	int_fast32_t currentOfs() const { return currentOfs_;  }
	DataPtr currentPtr() const { return pTile_ + currentOfs_; }

protected:
	DataPtr pTile_;
	int_fast32_t ofs_;
	int_fast32_t currentOfs_;
	int32_t rel_;
	int32_t tipDelta_;
};

