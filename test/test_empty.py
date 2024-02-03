from geodesk import *

def test_empty(features):
    empty = features.nodes.relations
    assert not empty
    assert empty.count == 0
    assert not empty("a")
    # assert not empty(Box(n=50, s=-50, w=-100, e=100))
    assert not empty.nodes
    for f in empty:
        pass
    streets=features("w[highway]")
    routes=features("r[route]")
    assert not streets & empty
    assert not routes & empty
    assert not empty.members_of(streets.first)
    assert not empty.members_of(routes.first)
    assert not empty.parents_of(streets.first)
    