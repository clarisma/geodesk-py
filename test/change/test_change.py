from shapely import *
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
    print (w1.tags)
    print (w1.nodes)

    r1 = changes.create([n1,w1], "route", "bus")
    print (r1.tags)
    print (r1.members)
    assert r1.is_relation
    assert r1.members[0].is_node
    assert r1.members[1].is_way

    w1[1:1] = [n1,n1,n1,n1]
    assert w1[2] == n1
    assert w1[3] == n1
    print(w1.nodes)

    # changes.save("d:\\geodesk\\tests\\changes")

def test_change_tags(monaco):
    f = monaco("na[amenity=restaurant]").first
    changes = Changes()
    cf = changes[f]
    cf.tags = { "cuisine": ["pizza","sushi"], "takeout": True }
    assert cf["cuisine"] == "pizza;sushi"
    assert cf["takeout"] == "yes"
    cf["cuisine"] = "french"
    assert cf.cuisine == "french"
    cf.takeout = False
    assert cf["takeout"] == "no"
    assert "cuisine" in cf.tags
    del cf["cuisine"]
    assert "cuisine" not in cf.tags
    assert cf["cuisine"] is None
    cf.takeout = ""
    assert "takeout" not in cf.tags
    assert cf["takeout"] is None
    cf.cuisine = ("italian", "french")
    assert cf.cuisine == "italian;french"
    cf.cuisine = ()
    assert "cuisine" not in cf.tags
    cf.cuisine = 13.750
    assert cf.tags["cuisine"] == "13.75"
    del cf.cuisine
    assert cf.cuisine is None


def test_create_polygon(monaco):
    outer = [[7.423662508,43.7301414031],[7.4224608777,43.7305962266],[7.4228328106,43.7312288382],[7.4233764051,43.7314727852],[7.4243205417,43.7314521115],[7.4249156346,43.7306003613],[7.423662508,43.7301414031]]
    inner = [[7.4238813763,43.7307678179],[7.4233892794,43.7309414764],[7.4236238843,43.7310820567],[7.4238298767,43.7310241705],[7.4238813763,43.7307678179]]
    p = Polygon(shell=outer, holes=[inner])
    changes = Changes()
    cf = changes.create(p)
    assert cf.is_relation
    assert len(cf.members) == 2
    assert cf[0].role == "outer"
    assert cf[1].role == "inner"

