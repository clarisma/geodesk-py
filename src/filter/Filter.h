// Copyright (c) 2023 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include <vector>
#include "feature/Feature.h"
#include "geom/Tile.h"
#include <common/util/RefCounted.h>

enum FilterFlags
{
    /**
     * Filter uses spatial index.
     *
     * If set, `Filter ` must be a subclass of `SpatialFilter`.
     */
    USES_BBOX = 1,

    /**
     * Features accepted by the Filter must be fully contained within the
     * bounding box (USES_BBOX must be set as well)
     */
    STRICT_BBOX = 2,

    /**
     * Given a specific tile, the Filter is able to accept all features
     * within this tile, reject the tile entirely, or at least offer
     * a simplified filter.
     *
     * If set, `Filter` must implement `acceptTile()`
     */
    FAST_TILE_FILTER = 4,

    /**
     * Filter is of class `ComboFilter`.
     *
     * If set, `Filter` must implement `acceptTile()`
     */
    COMBO_FILTER = 8,

    /**
     * Filter can be iterated directly (i.e. it has an implied target).
     * For example: around(50, feature) returns all features within 50 meters
     * of the given feature.
     */
    ITERABLE = 16,      // TODO: remove?

    /**
     * If the candidate feature is a non-area relation, all of its members
     * must be accepted by this filter in order for the filter to match.
     */
    MUST_ACCEPT_ALL_MEMBERS = 32,
};


struct FastFilterHint
{
    uint32_t turboFlags;
    Tile tile;

    FastFilterHint() : turboFlags(0) {}
    FastFilterHint(uint32_t f, Tile t) : turboFlags(f), tile(t) {}
};


class Filter : public RefCounted
{
public:
    Filter() : flags_(0), acceptedTypes_(FeatureTypes::ALL) {}
    Filter(int flags, FeatureTypes acceptedTypes) :
        flags_(flags), acceptedTypes_(acceptedTypes) {}
    virtual ~Filter() {};

    int flags() const { return flags_; }
    bool isCombo() const { return flags_ & FilterFlags::COMBO_FILTER; }
    Box getBounds() const;
    FeatureTypes acceptedTypes() const { return acceptedTypes_; }

    virtual bool accept(FeatureStore* store, FeatureRef feature, FastFilterHint fast) const
    {
        return false;
    }

    /**
     * Returns:
     *  0 if normal filtering applies
     *  1 if tile is eligible for acceleration
     *    (ComboFilter returns a bit pattern indicating fast-filter status
     *     of each child filter)
     * -1 if tile should be skipped altogether
     * 
     * For ComboFilter, tile fails to fulfill any filter, it fails for the entire chain.
     */
    virtual int acceptTile(Tile tile) const
    {
        return 0;
    }

protected:
    int flags_;
	FeatureTypes acceptedTypes_;
};





