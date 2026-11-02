from geodesk import *

world = Features("d:\\geodesk\\tests\\france.gol")
houses = world("a[building=house]").min_area(2000)

m = Map(link="https://www.osm.org/edit?{osm_type}={id}", color="red")
for area in world("a[landuse=industrial]"):
    for house in houses.within(area):
        m.add(house)
m.show()