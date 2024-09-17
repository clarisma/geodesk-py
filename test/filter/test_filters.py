# Copyright (c) 2024 Clarisma / GeoDesk contributors
# SPDX-License-Identifier: LGPL-3.0-only

from re import A
import time
from geodesk import *

class Stopwatch:
    def __init__(self):
        pass
    
    def start(self, task):
        self.start_time = time.time()
        self.task = task
        
    def stop(self):
        delta = round(time.time() - self.start_time, 3)
        print(f"{self.task}: {delta} s")


def notest_intersecting_and_within(features):
    s = Stopwatch()
    s.start("Fetch country")
    country = features("a[boundary=administrative][admin_level=2][name:en=Germany]")[0]
    s.stop()
    buildings = features("a[building=yes]")
    intersecting = buildings.intersecting(country)
    within = buildings.within(country)
    s.start("Count buildings that intersect country")
    print (intersecting.count)
    s.stop()
    s.start("Count buildings within country")
    print (within.count)
    s.stop()
    s.start("Create set (intersecting)")
    # a = { f.id for f in intersecting }
    a = set(intersecting)
    s.stop()
    print(len(a))
    s.start("Create set (within)")
    # b = { f.id for f in within }
    b = set(within)
    s.stop()
    print(len(b))
    s.start("Create differential set")
    diff = a - b
    s.stop()
    print(len(diff))
    country.map.add(diff, color="red").show()    
    
def test_nearby_hotels(features):
    state = features("a[boundary=administrative][admin_level=4][name:en=Bavaria]").one
    bike_routes = features("r[route=bicycle][network=ncn]").intersecting(state)
    hotels = features("na[tourism=hotel,guest_house,hostel]")
    m = Map()
    s = bike_routes.shape
    print("A")
    s.simplify(50000)
    print("B")
    s = s.buffer(100000)
    print("C")
    m.add(s)    
    m.add(bike_routes, color="red")
    m.add(hotels.within(s), color="orange")
    m.show()
    
def notest_within_geometry(features):
    state = features("a[boundary=administrative][admin_level=4][name:en=Bavaria]").one
    hotels = features("na[tourism=hotel,guest_house,hostel]")
    print(f"{hotels.within(state).count} hotels in {state.name}")
    s = state.shape
    print("Obtained shape")
    print(f"{hotels.within(s).count} hotels in {state.name} (shape)")
    