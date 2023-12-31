# Finds islands that are contained within other islands

from geodesk import *

planet = Features("c:\\geodesk\\tests\\w2.gol")

islands = planet("a[place=island,islet]")
island_count = 0
inner_island_count = 0
for outer_island in islands.relations:
    if outer_island.id in {
        15441662, 15763992, 12691338, 12125559, 12288209, 10791297, 10792714 }:
        print(f"Checking island: {outer_island}")
        print(outer_island.wkt)
    else:    
        print(f"Checking island: {outer_island}")
        inner_islands = islands.within(outer_island)
        inner_island_count += inner_islands.count
        island_count += 1
print(f"Found {island_count} islands with {inner_island_count} inner islands.")
