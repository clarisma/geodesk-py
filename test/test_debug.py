from geodesk2 import *

def test_debug():
    world = Features("data/monaco")
    w = world.way(899409587)
    print(w.nodes("n").count)
