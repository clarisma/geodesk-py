from geodesk import *

drammen = Features("c:\\geodesk\\tests\\drammen")
routes = drammen("r[route=bicycle]")
streets = drammen("w[highway][highway != path,footway,pedestrian,service]")

map = Map("cycle-routes")
for route in routes:
    print(f"{route} {route.name or 'Unnamed route'} ({route.ref}):")
    count = 0
    for member in route.members.ways:
        map.add(member, opacity=0.3, weight=20)
        for node in member.nodes:
            has_intersection = False
            for way in streets.parents_of(node):
                if not route in way.parents:
                    print(f"  - Intersects with {way.highway} {way.name or 'without name'}")
                    has_intersection = True
            if has_intersection:
                map.add(node, color="red")
                count += 1
    print(f"  {count} intersections.")
map.show()

