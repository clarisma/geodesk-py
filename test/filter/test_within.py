# Copyright (c) 2024 Clarisma / GeoDesk contributors
# SPDX-License-Identifier: LGPL-3.0-only

from shapely import *
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
    print(
        "(This total may be higher if there are restaurants that straddle\n"
        "a state boundary, and hence are not *within* either state)")

# An area is considered to be within itself
def notest_area_within_itself(features):
    # areas = features("a[wikidata=Q54150]")
    areas = features("a")
    for a in areas:
        # print(f"Checking {a}...")
        assert a in features.within(a)


def test_within_polygon(features):
    coords = [
        (11.5760065725, 48.1248103897),
        (11.5553563344, 48.1343158898),
        (11.5703099587, 48.1446341373),
        (11.5763117367, 48.1383212436),
        (11.5866877313, 48.1423262866),
        (11.5946223132, 48.1307175892),
        (11.5760065725, 48.1248103897)  # Close the polygon
    ]
    polygon = Polygon(to_mercator(coords))
    Map().add(features("n[amenity]")(polygon)).show()


