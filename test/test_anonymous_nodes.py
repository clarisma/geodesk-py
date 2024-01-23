from geodesk import *

def check_anon_nodes(nodes):
    assert nodes.shape is not None
    assert nodes.length == 0
    assert nodes.area == 0
    for node in nodes:
        c = Coordinate(node.x, node.y)
        assert c == node.centroid
        assert c.lon == node.lon
        assert c.lat == node.lat
        assert node.area == 0
        assert "Point" in str(node.geojson)
        assert node.length == 0
        assert node.members.count == 0
        assert node.nodes.count == 0
        assert "POINT" in str(node.wkt)
        
    
def test_anonymous_nodes(features):
    any_anon_nodes = False
    for way in features.ways:
        has_anon_nodes = False
        for node in way.nodes:
            if node.id == 0:
                has_anon_nodes = True
                break
        if has_anon_nodes:
            # print(f"Checking {way}...")
            check_anon_nodes(way.nodes)
            any_anon_nodes = True
            break
    if not any_anon_nodes:
        raise RuntimeError("No ways with anonymous nodes found!")
    

    