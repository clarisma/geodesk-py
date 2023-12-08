

def test_18(features):
    nodes = features("n")
    areas = features("a")

    for area in areas.members_of(nodes[0]):
        print(area)
        
    for area in areas.nodes_of(nodes[0]):
        print(area)

