# Finds islands that are contained within other islands

from geodesk import *

planet = Features("c:\\geodesk\\tests\\world.gol")
islands = planet("a[place=island,islet]")
for outer_island in islands:
    print(f"Checking island: {outer_island}")
    inner_islands = islands.within(outer_island)
