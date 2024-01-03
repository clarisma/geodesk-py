# Finds islands that are contained within other islands

from geodesk import *

planet = Features("c:\\geodesk\\tests\\w2.gol")

islands = planet("a[place=island,islet]")
map = Map()
count = 0
for outer_island in islands.relations:
    inner_islands = islands.within(outer_island)
    if inner_islands:
        # print(f"{outer_island} has inner islands")
        # map.add(outer_island, color="orange")
        for inner in inner_islands:
            if inner == outer_island:
                print(f"{outer_island} is within itself")
            # map.add(inner, color="red", tooltip="{name}", link="https://www.openstreetmap.org/{osm_type}/{id}")
        count += 1
        if count > 1000:
            break
# map.show()