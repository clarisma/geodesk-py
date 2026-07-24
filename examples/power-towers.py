import geodesk

world = geodesk.Features("d:\\geodesk\\tests\\de")
towers = world("n[power=tower]")
for tower in towers:
    nearby = list(towers.around(tower, meters=10))
    other_count = len(nearby) - 1
    if other_count:
        print(f"{tower} is near {other_count} "
            f"other tower{'s' if other_count > 1 else ''}:")
        for other in nearby:
            if other != tower:
                print(f"- {other}")



