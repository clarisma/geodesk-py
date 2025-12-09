from geodesk import *

world = Features("d:\\geodesk\\tests\\de")

coords = set()
total_nodes = 0
total_way_nodes = 0
for n in world.nodes:
    coords.add(n.centroid)
    total_nodes += 1
print(f"Total feature nodes:        {total_nodes}")
print(f"  Unique locations:         {len(coords)}")
for w in world.ways:
    for n in w.nodes:
        coords.add(n.centroid)
        total_way_nodes += 1
print(f"Total way-node references:  {total_way_nodes}")
print(f"Unique node locations:      {len(coords)}")

