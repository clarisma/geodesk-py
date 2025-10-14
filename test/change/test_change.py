from geodesk import *

def test_change(features):
    f = features("w[highway]").first
    changes = Changes()
    print(changes[f].tags)
    nodes = changes[f].nodes
    print(nodes)
    nodes.append("banana")
    print(nodes)
    print(nodes[0].lon)
    nodes[0].lon = 179.5
    print(nodes[0].lon)

