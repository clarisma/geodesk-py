// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#include <catch2/catch_test_macros.hpp>
#include "geom/TilePair.h"

void testAddTiles(const char* a, const char* b, const char* res)
{
    TilePair pair;
    Tile aTile = Tile::fromString(a);
    Tile bTile = Tile::fromString(b);
    pair += aTile;
    pair += bTile;

    CAPTURE(a, b, res, pair.toString());
    REQUIRE(pair.toString() == res);
}

TEST_CASE("Adding Tiles to a TilePair") 
{
    testAddTiles("2/3/2", "12/1/1", "0/0/0");
    testAddTiles("1/0/1", "12/1/1", "1/0/0+");
    testAddTiles("1/1/0", "12/1/1", "1/0+/0");
    testAddTiles("10/540/330", "12/2160/1322", "10/540/330");
    testAddTiles("10/540/330", "12/2160/1324", "10/540/330+");
    testAddTiles("12/2160/1322", "10/540/330", "10/540/330");
    testAddTiles("12/2160/1322", "12/2163/1322", "11/1080+/661");
}


/*
TEST_CASE("Twins of a TilePair")
{
    TilePair pair(Tile::fromString("10/536/371"));
    Tile tile = Tile::fromString("10/536/370");
}
*/