from geodesk import *

world = Features("d:\\geodesk\\tests\\world")
admin_areas = world("a[boundary=administrative]")
for n in range(1,100):
    print(f"Run {n}")
    pt = lonlat(8.12, 50.13)
    for area in admin_areas.containing(pt):
        print(f"- {area.name} at level {area.admin_level}")
