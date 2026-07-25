# Copyright (c) 2024 Clarisma / GeoDesk contributors
# SPDX-License-Identifier: LGPL-3.0-only

import geodesk
import json
import shapely
from shapely.geometry import shape

def test_intersecting(world):
    """
    - An `intersects` query should return the same number of features,
     regardless whether we use the area feature or its shape
    - The short form `features(area)` creates an `intersects` filter
      under the hood, so those results should be the same as well
    """

    city = world("a[boundary=administrative][admin_level=6][name=Braunschweig]").one
    streets = world("w[highway]")
    count = streets.intersecting(city).count
    assert count > 100
    assert streets.intersecting(city.shape).count == count
    assert streets(city).count == count
    assert streets(city.shape).count == count
    in_city = world.intersecting(city)
    assert streets(in_city).count == count
    assert (streets & in_city).count == count
    
def test_intersecting_contains_mp(features):
    """
    A candidate feature intersects a test geometry that is a multipolygon,
    even if the candidate only contains one of the test MP's polygons.
    (Issue #31)
    """
    geojson_string = '{"type": "MultiPolygon", "coordinates": [[[[1.130858, 47.665071], [0.0761706, 46.9955999], [1.4824205, 46.2265974], [2.7568345, 46.5445679], [2.9326157, 47.2196999], [2.2294907, 48.0333352], [1.130858, 47.665071]]], [[[11.5492744, 48.1454669], [11.5410347, 48.1408897], [11.553566, 48.1373421], [11.5492744, 48.1454669]]]]}'
    geojson_dict = json.loads(geojson_string)
    geom_wgs84 = shape(geojson_dict)
    geom = geodesk.to_mercator(geom_wgs84)
    # print(f"Original test shape: {geom_wgs84}")
    # print(f"Mercator test shape: {geom}")
    assert geom_wgs84.is_valid
    assert geom.is_valid
    first_geom = geom.geoms[0]      # the part in France
    second_geom = geom.geoms[1]     # the part near Munich
    bavaria = features("a[boundary=administrative][admin_level=4][name:en=Bavaria]")
    bavaria_shape = bavaria.one.shape
    assert bavaria_shape.is_valid
    pt = shapely.Point(geodesk.lonlat(11.5492744, 48.1454669))  # point near munich
    assert bavaria_shape.intersects(pt)
    assert pt.within(bavaria_shape)
    assert not bavaria_shape.intersects(first_geom)
    for vertex in second_geom.exterior.coords:
        assert shapely.Point(vertex).within(bavaria_shape)

    second_clean = shapely.from_wkb(shapely.to_wkb(second_geom))
    bavaria_clean = shapely.from_wkb(shapely.to_wkb(bavaria_shape))
    assert second_clean.is_valid
    assert bavaria_clean.is_valid
    assert second_clean.intersects(bavaria_clean)
    assert second_clean.intersects(bavaria_shape)
    assert second_geom.intersects(bavaria_clean)

    assert bavaria.intersecting(second_geom)
    assert not bavaria.intersecting(first_geom)
    assert second_geom.covered_by(bavaria_shape)
    assert second_geom.within(bavaria_shape)
    assert bavaria_shape.intersects(second_geom)
    assert geom.intersects(bavaria_shape)
    assert bavaria_shape.intersects(geom)
    assert bavaria.intersecting(geom)
    
def test_intersecting_waynode(features):
    """
    Each parent way of a node by definition must intersect the node.
    (Issue #44)
    """
    streets = features("w[highway]")
    for street in streets[:100]:
        for node in street:
            # print(f"Checking {node}:")
            intersecting_features = features.intersecting(node)
            intersecting_streets = streets.intersecting(node)
            parent_ways = node.parents.ways
            # print(f"  {parent_ways.count} parents")
            # print(f"  {parent_ways.count} parents")
            # print(f"  {intersecting_features.count} intersecting features")
            # print(f"  {intersecting_streets.count} intersecting streets")
            assert street in intersecting_features
            assert street in intersecting_streets
            for parent in parent_ways:
                assert parent in intersecting_features
                                