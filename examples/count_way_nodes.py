from geodesk import *

world = Features('c:\\geodesk\\tests\\w2.gol')
total_ways = 0
total_ways_with_nodes = 0
total_way_nodes = 0

for way in world.ways: # ("w[highway]"):
    total_ways += 1
    node_count = way.nodes("n").count
    if node_count:
        total_ways_with_nodes += 1
        total_way_nodes += node_count

print(f"{total_way_nodes} in {total_ways_with_nodes} ways (of {total_ways})")
    
    