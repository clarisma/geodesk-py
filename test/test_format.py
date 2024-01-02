# Copyright (c) 2024 Clarisma / GeoDesk contributors
# SPDX-License-Identifier: LGPL-3.0-only

def notest_geojson(features):
    city = features("a[boundary=administrative][admin_level=6][name:en=Munich]").one
    features("n[amenity=post_box]").within(city).geojson.save(
        "c:\\geodesk\\tests\\postboxes")
    routes = features("r[route=bicycle]").intersects(city).geojson.save(
        "c:\\geodesk\\tests\\routes")
    
def test_wkt(features):
    for f in features[:100]:
        print(f.wkt)