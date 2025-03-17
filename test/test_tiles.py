# Copyright (c) 2024 Clarisma / GeoDesk contributors
# SPDX-License-Identifier: LGPL-3.0-only

from geodesk import *

def test_world_tiles():
    world = Features("c:\\geodesk\\tests\\w.gol")
    # country = world("a[boundary=administrative][admin_level=2][name:en=Germany]").one
    # country = world("a[boundary=administrative][admin_level=4][name:en=Bavaria]").one
    # country = world("a[boundary=administrative][admin_level=2][name='United States']").one
    country = world("a[boundary=administrative][admin_level=2][name:en=Thailand]").one
    m = Map()
    count = 0
    total_size = 0
    for tile in world(country).tiles:
        bounds = tile.bounds
        m.add(bounds, tooltip=f"{tile}: {tile.size / 1024 / 1024:.3} MB")
        count += 1
        total_size += tile.size    
    m.show()    
    print (f"{count} tiles")
    print (f"Tile set size: {total_size / 1024 / 1024} MB")    
    
def test_member_tiles():
    world = Features("c:\\geodesk\\tests\\w3.gol")
    country = world("a[boundary=administrative][admin_level=2][name:en=Germany]").one
    # country = world("a[boundary=administrative][admin_level=2][name='United States']").one
    m = Map()
    count = 0
    for tile in country.members.tiles:
        bounds = tile.bounds
        m.add(bounds, tooltip=str(tile))
        count += 1
    m.show()    
    print (f"{country.members.count} members in {count} tiles")
    