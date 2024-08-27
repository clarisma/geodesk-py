// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#include "ParentRelationIterator.h"
#include "FeatureStore.h"
#include "filter/Filter.h"


ParentRelationIterator::ParentRelationIterator(FeatureStore* store, pointer pRelTable,
    const MatcherHolder* matcher, const Filter* filter) :
    store_(store),
    matcher_(matcher),
    filter_(filter),
    p_(pRelTable),
    currentTip_(FeatureConstants::START_TIP),
    pForeignTile_(nullptr),
    currentRel_(0)
{
}

RelationRef ParentRelationIterator::next()
{
    while ((currentRel_ & MemberFlags::LAST) == 0)
    {
        pointer pCurrent = p_;
        currentRel_ = p_.getInt();
        p_ += 4;
        RelationRef rel(nullptr);
        if (currentRel_ & MemberFlags::FOREIGN)
        {
            if (currentRel_ & MemberFlags::DIFFERENT_TILE)
            {
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
                pForeignTile_ = store_->fetchTile(currentTip_);
            }
            rel = RelationRef(pForeignTile_ + ((currentRel_ & 0xffff'fff0) >> 2));
        }
        else
        {
            assert((pCurrent.pointerAsULong() & 1) == 0); // Reltable must be 2-byte aligned
            rel = RelationRef(pCurrent + ((int32_t)(currentRel_ & 0xffff'fffc) >> 1));
                // TODO: shift should not be needed, flaw in Tile Spec 1.0
        }
        if (matcher_->mainMatcher().accept(rel.ptr()))
        {
            if (filter_ == nullptr || filter_->accept(store_, rel, FastFilterHint()))
            {
                return rel;
            }
        }
    }
    return RelationRef(nullptr);
}

