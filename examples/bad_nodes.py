import sys
from geodesk import *

if len(sys.argv) < 3:
    print("Must specify a 2 GOQL queries: way and node")
    sys.exit()

world = Features("c:\\geodesk\\tests\\fr.gol")
ways = world(sys.argv[1]).ways
nodes = world(sys.argv[2]).nodes

m = Map(link="http://www.osm.org/edit?{osm_type}={id}")
for way in ways:
    marked_way = False
    for node in nodes.nodes_of(way):
        if not marked_way:
            m.add(way, color="red", tooltip=str(way.tags))
            marked_way = True
        # print("Tooltip is " + str(node.tags))    
        m.add(node, color="orange", tooltip="Test")
m.show()
            
        