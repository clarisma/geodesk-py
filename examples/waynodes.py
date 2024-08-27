from geodesk import *

world = Features("c:\\geodesk\\tests\\w3.gol")
way_count = 0
single_waynode_count = 0
multi_waynode_count = 0

for way in world.ways:
    way_count += 1
    node_count = way.nodes("n").count
    if node_count > 0:
        if node_count > 1:
            multi_waynode_count += 1
        else:
            single_waynode_count += 1

print(f"Of {way_count} ways:")            
print(f"- {single_waynode_count} have 1 feature node")            
print(f"- {multi_waynode_count} have 2+ feature nodes")            
    