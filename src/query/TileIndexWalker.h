// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include <unordered_set>
#include "feature/FeatureStore.h"
#include "geom/Box.h"
#include "geom/Tile.h"

class Filter;

class TileIndexWalker
{
public:
    TileIndexWalker(pointer pIndex, uint32_t zoomLevels, 
        const Box& box, const Filter* filter);

    bool next();
    uint32_t currentTip() const { return currentTip_; }
    Tile currentTile() const { return currentTile_; }
    uint32_t northwestFlags() const { return northwestFlags_; }
    const Box& bounds() const { return box_; }

private:
    static const int MAX_LEVELS = 13;   // currently 0 - 12

    // TODO: uint16 supports max level 15 (not 16, because of sign;
    //  we start at columns at -1)
	struct Level
	{
		uint64_t childTileMask;
        int32_t pChildEntries;
        Tile topLeftChildTile;
        uint16_t step;       // difference in zoom level to parent
        int16_t startCol;
        int16_t endCol;
        int16_t endRow;
        int16_t currentCol;
        int16_t currentRow;
        uint32_t turboFlags;
	};

    void startLevel(Level* level, int tip);
    void startRoot();
    
    Box box_;
    const Filter* filter_;
    pointer pIndex_;
    int currentLevel_;
    Tile currentTile_;
    uint32_t currentTip_;
    bool tileBasedAcceleration_;
    bool trackAcceptedTiles_;
    uint32_t northwestFlags_;
    std::unordered_set<Tile> acceptedTiles_;
    Level levels_[MAX_LEVELS];
};