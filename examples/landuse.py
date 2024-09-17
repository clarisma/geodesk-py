from geodesk import *

world = Features('c:\\geodesk\\tests\\de3.gol')
landuse_areas=world("a[landuse]")
for area in landuse_areas.containing(latlon(49.64001, 8.48114)):
    print(area.landuse)                          

