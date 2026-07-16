# Copyright (c) 2024 Clarisma / GeoDesk contributors
# SPDX-License-Identifier: LGPL-3.0-only

def test_strings(features):
    # print(features.strings)
    strings = features.strings
    assert strings[0] == ""
    assert "no" in strings
    assert len(strings) > 255
    # There must not be any duplicates
    assert len(strings) == len(set(strings))
    
def test_indexed_keys(features):
    # print(features.indexed_keys)
    indexed_keys = features.indexed_keys
    assert "building" in indexed_keys
    assert "highway" in indexed_keys
    # There must not be any duplicates
    assert len(indexed_keys) == len(set(indexed_keys))
    
def notest_tile_size(features):
    tile_count = 0 
    total_size = 0
    for tile in features.tiles:
        size = tile.size
        print(f"{tile}: {size} bytes")
        total_size += size
        tile_count += 1
    print(f"{tile_count} tiles with {total_size} bytes total")

def test_revision(world):
    assert world.revision > 0

def test_tiles(world):
    tiles = world.tiles
    str_tiles = [str(t) for t in tiles]
    assert "0/0/0" in str_tiles
    assert "4/8/5" in str_tiles
    total_size = 0
    for tile in tiles:
        total_size += tile.size
    assert len(tiles) > 25000
    assert total_size > 90 * 1024 * 1024 * 1024     # world should be > 90 GB
