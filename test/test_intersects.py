# Copyright (c) 2024 Clarisma / GeoDesk contributors
# SPDX-License-Identifier: LGPL-3.0-only

import geodesk
import json
from shapely.geometry import shape

def test_intersects(features):
    """
    - An `intersects` query should return the same number of features,
     regardless whether we use the area feature or its shape
    - The short form `features(area)` creates an `intersects` filter
      under the hood, so those results should be the same as well
    """

    city = features("a[boundary=administrative][admin_level=6][name=Braunschweig]").one
    streets = features("w[highway]")
    count = streets.intersects(city).count
    assert count > 100
    assert streets.intersects(city.shape).count == count
    assert streets(city).count == count
    assert streets(city.shape).count == count
    in_city = features.intersects(city)
    assert streets(in_city).count == count
    assert (streets & in_city).count == count
    
def test_intersects_contains_mp(features):
    """
    A candidate feature intersects a test geometry that is a multipolygon,
    even if the candidate only contains one of the test MP's polygons.
    (Issue #31)
    """
    geojson_string = '{"type": "MultiPolygon", "coordinates": [[[[1.130858, 47.665071], [0.0761706, 46.9955999], [1.4824205, 46.2265974], [2.7568345, 46.5445679], [2.9326157, 47.2196999], [2.2294907, 48.0333352], [1.130858, 47.665071]]], [[[11.5492744, 48.1454669], [11.5410347, 48.1408897], [11.553566, 48.1373421], [11.5492744, 48.1454669]]]]}'
    geojson_dict = json.loads(geojson_string)
    geom = geodesk.to_mercator(shape(geojson_dict))
    bavaria = features("a[boundary=administrative][admin_level=4][name:en=Bavaria]")
    assert geom.intersects(bavaria.one.shape)
    assert bavaria.one.shape.intersects(geom)
    assert bavaria.intersects(geom)
    
def test_intersects_waynode(features):
    """
    Each parent way of a node by definition must intersect the node.
    (Issue #44)
    """
    streets = features("w[highway]")
    for street in streets[:100]:
        for node in street:
            # print(f"Checking {node}:")
            intersecting_features = features.intersects(node) 
            intersecting_streets = streets.intersects(node) 
            parent_ways = node.parents.ways
            # print(f"  {parent_ways.count} parents")
            # print(f"  {parent_ways.count} parents")
            # print(f"  {intersecting_features.count} intersecting features")
            # print(f"  {intersecting_streets.count} intersecting streets")
            assert street in intersecting_features
            assert street in intersecting_streets
            for parent in parent_ways:
                assert parent in intersecting_features
                                