from geodesk import *

"""
def test_debug():
    world = Features("data/monaco")
    w = world.way(899409587)
    crossings = world("n[highway=crossing]")
    assert crossings.nodes_of(w).count == 2
"""

"""
def test_debug():
    strings = set()
    world = Features("data/monaco")
    for f in world:
        for k,v, in f.tags:
            strings.add(f"{f}: {f.str(k)}")
    l = list(strings)
    l.sort()
    with open("d:\\geodesk\\tests\\monaco-py.txt", "w", encoding="utf-8") as f:
        for s in l:
            f.write(s + "\n")
"""

def test_debug2():
    world = Features("data/monaco")
    f=world.node(9983257451)
    print(f.tags)
    print(f.str("stop"))


def test_debug_ints():
    strings = list()
    world = Features("data/monaco")
    for f in world:
        for k,v, in f.tags:
            n = int(f.num(k))
            strings.append(f"{f}: {n}")
    strings.sort()
    with open("d:\\geodesk\\tests\\monaco-ints-py.txt", "w", encoding="utf-8") as f:
        for s in strings:
            f.write(s + "\n")
