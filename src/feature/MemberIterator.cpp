// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#include "MemberIterator.h"
#include <common/util/log.h>
#include "FeatureStore.h"
#include "filter/Filter.h"

#ifdef GEODESK_TEST_PERFORMANCE
extern volatile uint32_t performance_blackhole;
#endif 

MemberIterator::MemberIterator(FeatureStore* store, pointer pMembers,
    FeatureTypes types, const MatcherHolder* matcher, const Filter* filter) : 
    store_(store),
    types_(types),
    matcher_(matcher),
    filter_(filter),
    p_(pMembers),
    currentTip_(FeatureConstants::START_TIP),
    currentRoleCode_(0),
    currentRoleStr_(nullptr),
    pForeignTile_(nullptr)
{
    // check for empty relation
    currentMember_ = p_.getUnalignedInt() == 0 ? MemberFlags::LAST : 0;
    currentMatcher_ = &matcher->mainMatcher(); // TODO: select based on role
    #ifdef GEODESK_PYTHON
    // currentRoleObject_ = store->strings().getStringObject(0);
        // TODO: this bumps the refcount; let's use a "borrow" function instead!
        // TODO: check for refcount handling in code below
        // No, see comments below -- must refcount because local strings are 
        // treated as owned
    // assert(currentRoleObject_);
    currentRoleObject_ = nullptr;
    #endif
}

FeatureRef MemberIterator::next()
{
	while ((currentMember_ & MemberFlags::LAST) == 0)
	{
        pointer pCurrent = p_;
		currentMember_ = p_.getUnalignedInt();
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
            p_ += 2;
            if (rawRole & 1)
            {
                // common role
                currentRoleCode_ = rawRole >> 1;   
                // TODO: ensure this will be unsigned
                currentRoleStr_ = nullptr;
                #ifdef GEODESK_PYTHON
                if (currentRoleObject_)
                {
                    Py_DECREF(currentRoleObject_);
                    currentRoleObject_ = nullptr;
                }
                    // move out of if?
                // TODO: This is wrong, needs to borrow!
                //  No, it has to addref, because refs to local strings are owned
                //  and we have no way to distinguish between them that is cheaper
                //  than refcounting
                // currentRoleObject_ = store_->strings().getStringObject(currentRoleCode_);
                // assert(currentRoleObject_);
                #endif
            }
            else
            {
                rawRole |= static_cast<int>(p_.getShort()) << 16;
                currentRoleCode_ = -1;
                currentRoleStr_ = p_ + ((rawRole >> 1) - 2); // signed
                #ifdef GEODESK_PYTHON
                if (currentRoleObject_)
                {
                    Py_DECREF(currentRoleObject_);
                    currentRoleObject_ = nullptr;
                }
                // currentRoleObject_ = currentRoleStr_.toStringObject();
                // assert(currentRoleObject_);
                #endif
                p_ += 2;
            }
            // TODO: we may increase efficiency by only fetching the currentRoleStr_
            // if the role is accepted by the matcher, but this design is simpler
             
            // TODO: get matcher for this new role
            // (null if role is not accepted)
            // currentMatcher = matcher.acceptRole(role, roleString);
        }
        
        if (currentMatcher_ != nullptr)
        {
            // Current member's role is acceptable
            // (i.e. currentMatcher_ is not null)

            FeatureRef feature(nullptr);
            if (currentMember_ & MemberFlags::FOREIGN)
            {
                if (!pForeignTile_)
                {
                    // foreign tile not resolved yet
                    pForeignTile_ = store_->fetchTile(currentTip_);
                }
                feature = FeatureRef(pForeignTile_ + 
                    ((currentMember_ & 0xffff'fff0) >> 2));

#ifdef GEODESK_TEST_PERFORMANCE
                // Simulate the extra lookup needed to retrieve a foreign
                // feature via an export table, rather than directly    
                uint32_t tileSize = pForeignTile_.getUnsignedInt() & 0x3fff'ffff;
                uint64_t id = feature.id();
                uint32_t simulatedSlotNumber = id % 16'000;
                pointer pSimulatedExportSlot = pForeignTile_ + 4 * 4096 + simulatedSlotNumber * 4;
                if (pSimulatedExportSlot.asBytePointer() > pForeignTile_ + tileSize)
                {
                    pSimulatedExportSlot = pForeignTile_ + tileSize - 4;
                }
                /*
                pointer pSimulatedExportSlot = pForeignTile_ +
                    (feature.ptr() - pForeignTile_) / 8;
                */
                uint32_t simulatedSlotPtr = pSimulatedExportSlot.getUnsignedInt();
                performance_blackhole += tileSize + simulatedSlotPtr;
#endif
            }
            else
            {
                /*
                LOG("pCurrent = 0x%016llx", pCurrent);
                LOG("currentMember_ = 0x%08lx", currentMember_);
                LOG("currentMember_ = %d", currentMember_);
                LOG("masked currentMember_ = %d", (currentMember_ & 0xffff'fff8));
                LOG("shifted currentMember_ = %d", (currentMember_ >> 1));
                LOG("offset = %d", ((currentMember_ & 0xffff'fff8) >> 1));
                LOG("offset_v2 = %d", ((currentMember_ & 0xffff'ffff'ffff'fff8) >> 1));
                LOG("offset_v3 = %d", ((int32_t)(currentMember_ & 0xffff'fff8) >> 1));
                */

                feature = FeatureRef((pCurrent &
                    0xffff'ffff'ffff'fffcULL) +
                    ((int32_t)(currentMember_ & 0xffff'fff8) >> 1));
            }

            // Now test the candidate feature against the matcher and filter

            if (types_.acceptFlags(feature.flags()))
            {
                if (currentMatcher_->accept(feature.ptr()))
                {
                    if (filter_ == nullptr || filter_->
                        accept(store_, feature, FastFilterHint()))
                    {
                        // Feature accepted by currentMatcher_ and filter_
                        return feature;
                    }
                }
            }
        }
	}
#ifdef GEODESK_TEST_PERFORMANCE
    printf((performance_blackhole & 1) ? "odd\n" : "even\n");
#endif
    return FeatureRef(nullptr);
}
