from geodesk import *

def check_anon_nodes(nodes):
    assert nodes.shape is not None
    assert nodes.length == 0
    assert nodes.area == 0
    
def test_anonymous_nodes(features):
    any_anon_nodes = False
    for way in features.ways:
        has_anon_nodes = False
        for node in way.nodes:
            if node.id == 0:
                has_anon_nodes = True
                break
        if has_anon_nodes:
            check_anon_nodes(way.nodes)
            any_anon_nodes = True
            break
    if not any_anon_nodes:
        raise RuntimeError("No ways with anonymous nodes found!")