from geodesk import *

def test_change():
    features = Features("d:\\geodesk\\tests\\mcu")
    f = features("w[highway]").first
    changes = Changes()
    print(changes[f].tags)
    nodes = changes[f].nodes
    print(nodes)
    print(type(nodes))
    nodes.append("banana")
    print(nodes)
    print(nodes[0].lon)
    nodes[0].lon = 179.5
    print(nodes[0].lon)

    cf = changes[f]
    print(cf.tags)
    cf.tags = { "new_key": "new_value" }
    cf['cuisine'] = ['sushi','italian','frango']
    cf['flag'] = True
    cf['maxspeed'] = 50
    print(cf.tags)

    changes.save("d:\\geodesk\\tests\\changes")
