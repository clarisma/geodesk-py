// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include "feature/FeaturePtr.h"
#include "feature/Tex.h"
#include "feature/types.h"


class MemberTableIterator
{
public:
	MemberTableIterator(int_fast32_t handle, DataPtr pTable) :
		pTile_(pTable - handle),
		ofs_(handle),
		currentOfs_(0),
		currentRoleOfs_(0),
		member_(0),
		tipDelta_(0),
		rawRole_(1)
	{
	}

	bool next()
	{
		if (member_ & MemberFlags::LAST) return false;
		currentOfs_ = ofs_;
#ifdef GEODESK_READ_V2
		rel_ = p_.getUnsignedShort();
		// TODO
#else
		member_ = (pTile_ + ofs_).getIntUnaligned();
		ofs_ += 4;
#endif
		if ((member_ & (MemberFlags::FOREIGN | MemberFlags::DIFFERENT_TILE)) ==
			(MemberFlags::FOREIGN | MemberFlags::DIFFERENT_TILE))
		{
			// foreign member in different tile
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
		if (member_ & MemberFlags::DIFFERENT_ROLE)
		{
			// TODO: could read without branching if over-reading is allowed
			currentRoleOfs_ = ofs_;
			rawRole_ = (pTile_ + ofs_).getUnsignedShort();
			if (rawRole_ & 1)
			{
				ofs_ += 2;
			}
			else
			{
				rawRole_ = (pTile_ + ofs_).getIntUnaligned();
				ofs_ += 4;
			}
		}
		return true;
	}

	bool isForeign() const { return member_ & MemberFlags::FOREIGN; }
	bool isInDifferentTile() 
	{ 
		assert(isForeign());
		return member_ & MemberFlags::DIFFERENT_TILE;
	}

	bool hasDifferentRole() const { return member_ & MemberFlags::DIFFERENT_ROLE; }
	bool hasGlobalRole() const { return rawRole_ & 1; }
	bool hasLocalRole() const { return (rawRole_ & 1) == 0; }
	int32_t globalRoleFast() const
	{
		assert(hasGlobalRole());
		return rawRole_ >> 1;
	}
	int_fast32_t localRoleHandleFast() const
	{
		assert(hasLocalRole());
		// LOG("local string role = #%d", currentRoleOfs_ + (rawRole_ >> 1));
		return currentRoleOfs_ + (rawRole_ >> 1);
	}
	DataPtr localRoleStringFast() const
	{
		return pTile_ + localRoleHandleFast();
	}
	
	int_fast32_t localMemberHandle() const
	{
		return (currentOfs_ & 0xffff'fffc) + 
			(static_cast<int_fast32_t>(member_ & 0xffff'fff8) >> 1);
	}
	FeaturePtr localMember() const
	{
		return FeaturePtr(pTile_ + localMemberHandle());
	}

	TipDelta tipDelta() const { return TipDelta(tipDelta_); }
	TexDelta texDelta() const
	{
		return member_ >> 5;
	}


protected:
	DataPtr pTile_;
	int_fast32_t ofs_;
	int_fast32_t currentOfs_;
	int_fast32_t currentRoleOfs_;
	int32_t member_;			// must be signed for locals
	int32_t tipDelta_;
	int32_t rawRole_;
};

