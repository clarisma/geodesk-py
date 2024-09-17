// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#include "feature/FeatureNodeIterator.h"
#include "feature/FeatureStore.h"
#include "filter/Filter.h"

FeatureNodeIterator::FeatureNodeIterator(FeatureStore* store)
  : store_(store),
    currentTip_(FeatureConstants::START_TIP)
{
}


void FeatureNodeIterator::start(DataPtr pBody, int flags,
    const MatcherHolder* matcher, const Filter* filter)
{
    matcher_ = matcher;
    filter_ = filter;
    currentTip_ = FeatureConstants::START_TIP;
    pForeignTile_ = DataPtr();
    p_ = pBody - (flags & FeatureFlags::RELATION_MEMBER);
    currentNode_ = (flags & FeatureFlags::WAYNODE) ? 0 : MemberFlags::LAST;
}


NodePtr FeatureNodeIterator::next()
{
    while ((currentNode_ & MemberFlags::LAST) == 0)
    {
        p_ -= 4;
        DataPtr pCurrent = p_;
        currentNode_ = p_.getIntUnaligned();
        NodePtr feature(FeaturePtr(nullptr));
        if (currentNode_ & MemberFlags::FOREIGN)
        {
            if (currentNode_ & MemberFlags::DIFFERENT_TILE)
            {
                p_ -= 2;
                int32_t tipDelta = p_.getShort();
                if (tipDelta & 1)
                {
                    // wide TIP delta
                    p_ -= 2;
                    tipDelta = (tipDelta & 0xffff) |
                        (static_cast<int32_t>(p_.getShort()) << 16);
                }
                tipDelta >>= 1;     // signed
                currentTip_ += tipDelta;
                pForeignTile_ = store_->fetchTile(currentTip_);
            }
            feature = NodePtr(pForeignTile_ + ((currentNode_ & 0xffff'fff0) >> 2));
        }
        else
        {
            feature = NodePtr(pCurrent + (
                static_cast<int32_t>(currentNode_ & 0xffff'fffc) >> 1));
            // TODO: This may be wrong, pCurrent must be 4-byte aligned
            // In Java: pNode = (pCurrent & 0xffff_fffe) + ((node >> 2) << 1);
        }

        if(matcher_->mainMatcher().accept(feature))
        {
            // LOG("Node matched");
            if (filter_ == nullptr || filter_->accept(store_, feature, FastFilterHint()))
            {
                // Feature accepted by matcher_ and filter_
                return feature;
            }
        }
        else
        {
            // LOG("Node NOT matched");
        }
    }
    return NodePtr(FeaturePtr(nullptr));
}
