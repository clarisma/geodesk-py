import geodesk

world = geodesk.Features("d:\\geodesk\\tests\\w2")
lines = world("w[power=line]")
towers = world("n[power=tower]")
min_distance = 10

for line in lines:
    warned = False
    prev = None
    for tower in towers.nodes_of(line):
        if prev:
            d = tower.distance(prev)
            if d < min_distance:
                if not warned:
                    print(f"{line} has closely-placed towers:")
                    warned = True
                print(f"- {prev} and {tower} ({d:.2f} meters apart)")
        prev = tower
