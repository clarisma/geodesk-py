# Copyright (c) 2024 Clarisma / GeoDesk contributors
# SPDX-License-Identifier: LGPL-3.0-only

from geodesk import *

def test_within(features):
    country = features("a[boundary=administrative][admin_level=2][name:en=Germany]")[0]
    states = features("a[boundary=administrative][admin_level=4]")
    # bavaria = states[0]

    # map = Map()
    # map.add(country, color="blue")
    # map.add(bavaria, color="orange")
    # map.add(Coordinate(144368670, 696790469), color="red")
    # map.show()
    
    restaurants = features("na[amenity=restaurant]")
    # restaurants = features("a[building]")
    state_count = 0
    total_restaurant_count = 0
    for state in states.relations.within(country):
        restaurant_count = restaurants.within(state).count
        print(f"- {state.name} has {restaurant_count} restaurants")
        total_restaurant_count += restaurant_count
        state_count += 1
    print(f"{country.name} has {state_count} states")
    print(f"Sum of restaurants from state queries: {total_restaurant_count}")
    print(f"Total restaurants in {country.name}: {restaurants.within(country).count}")



