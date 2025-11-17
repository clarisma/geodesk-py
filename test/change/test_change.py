from geodesk import *

def test_change():
    features = Features("d:\\geodesk\\tests\\mcu")
    f = features("w[highway]").first
    changes = Changes()
    print(changes[f].tags)
    """
    nodes = changes[f].nodes
    print(nodes)
    print(type(nodes))
    # nodes.append("banana")        # TODO: test invalid assignment
    print(nodes)
    print(nodes[0].lon)
    nodes[0].lon = 179.5
    print(nodes[0].lon)
    """
    print("--- 1 ---")

    cf = changes[f]
    print("--- 2 ---")
    print(cf.tags)
    cf.tags = { "new_key": "new_value" }
    cf['cuisine'] = ['sushi','italian','frango']
    cf['flag'] = True
    cf['maxspeed'] = 50
    print(cf.tags)

    n1 = changes.create(14, 52)
    assert n1.is_node

    w1 = changes.create([(14, 52), (17, 56)], "highway", "primary")
    assert w1.is_way

    r1 = changes.create([n1,w1], "route", "bus")
    assert r1.is_relation

    # changes.save("d:\\geodesk\\tests\\changes")
