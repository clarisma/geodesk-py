# Copyright (c) 2024 Clarisma / GeoDesk contributors
# SPDX-License-Identifier: LGPL-3.0-only

from geodesk import *

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
