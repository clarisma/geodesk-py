// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#include "FastMemberIterator.h"
#include "FeatureStore.h"
#include <common/util/log.h>


FastMemberIterator::FastMemberIterator(FeatureStore* store, RelationPtr relation) :
    store_(store),
    p_(relation.bodyptr()),
    currentTip_(FeatureConstants::START_TIP)
    // pForeignTile_(nullptr)  // null by default
{
    // check for empty relation
    currentMember_ = p_.getIntUnaligned() == 0 ? MemberFlags::LAST : 0;
}

FeaturePtr FastMemberIterator::next()
{
    if (currentMember_ & MemberFlags::LAST) return FeaturePtr(nullptr);
    
    DataPtr pCurrent = p_;
    currentMember_ = p_.getIntUnaligned();
    p_ += 4;
    if (currentMember_ & MemberFlags::FOREIGN)
    {
        if (currentMember_ & MemberFlags::DIFFERENT_TILE)
        {
            pForeignTile_ = nullptr;
            int32_t tipDelta = p_.getShort();
            p_ += 2;
            if (tipDelta & 1)
            {
                // wide TIP delta
                tipDelta = (tipDelta & 0xffff) |
                    (static_cast<int32_t>(p_.getShort()) << 16);
                p_ += 2;
            }
            tipDelta >>= 1;     // signed
            currentTip_ += tipDelta;
        }
    }
    if (currentMember_ & MemberFlags::DIFFERENT_ROLE)
    {
        int rawRole = p_.getUnsignedShort();
        p_ += (rawRole & 1) ? 2 : 4;        
            // Bit 0 is global-code flag
            // If 0, means role is a local string (4-bytes instead of 2)
    }

    FeaturePtr feature(nullptr);
    if (currentMember_ & MemberFlags::FOREIGN)
    {
        if (!pForeignTile_)
        {
            // foreign tile not resolved yet
            pForeignTile_ = store_->fetchTile(currentTip_);
        }
        feature = FeaturePtr(pForeignTile_ +
            ((currentMember_ & 0xffff'fff0) >> 2));
    }
    else
    {
        feature = FeaturePtr(pCurrent.andMask(0xffff'ffff'ffff'fffc) +
            ((int32_t)(currentMember_ & 0xffff'fff8) >> 1));
    }
    return feature;
}
