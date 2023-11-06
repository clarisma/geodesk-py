from geodesk import *

def notest_world_tiles():
    world = Features("c:\\geodesk\\tests\\world.gol")
    germany = world("a[boundary=administrative][admin_level=2][name:en=Germany]").one
    m = Map()
    count = 0
    for tile in world(germany).tiles:
        bounds = tile.bounds
        m.add(bounds, tooltip=str(tile))
        count += 1
    m.show()    
    print (f"{count} tiles")
    
def test_member_tiles():
    world = Features("c:\\geodesk\\tests\\world.gol")
    germany = world("a[boundary=administrative][admin_level=2][name:en=Germany]").one
    m = Map()
    count = 0
    for tile in germany.members.tiles:
        bounds = tile.bounds
        m.add(bounds, tooltip=str(tile))
        count += 1
    m.show()    
    print (f"{germany.members.count} members in {count} tiles")