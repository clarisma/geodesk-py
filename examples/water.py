# Get the polygons of all water areas

from geodesk import *

world = Features('world.gol')
water_areas = world("a[natural=water]")
for water in water_areas:
    print(water.name)    # The feature's name
    print(water.shape)   # Its geometry
    print(water.area)    # Its size in square meters