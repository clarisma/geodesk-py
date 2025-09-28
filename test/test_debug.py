from geodesk2 import *

def test_debug():
    world = Features("data/monaco")
    w = world.way(899409587)
    crossings = world("n[highway=crossing]")
    assert crossings.nodes_of(w).count == 2
