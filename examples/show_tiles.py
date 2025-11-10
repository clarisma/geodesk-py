from geodesk import *

world = Features("d:\\geodesk\\tests\\world")
m = Map()
int_max = (1 << 31)-1
for tile in world.tiles:
    if tile.zoom <= 6:
        b = tile.bounds
        b2 = Box(b.minx, b.miny,
            min(b.maxx+1, int_max),
            min(b.maxy+1, int_max))
        m.add(b2, color="#111111", fillColor="#99ff99", weight=.5, opacity=1, fillOpacity=0.1)
m.show()

