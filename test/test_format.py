# Copyright (c) 2024 Clarisma / GeoDesk contributors
# SPDX-License-Identifier: LGPL-3.0-only

from geodesk import *
import json
import shapely.geometry
import shapely.wkt


def notest_geojson(features):
    city = features("a[boundary=administrative][admin_level=6][name:en=Munich]").one
    features("n[amenity=post_box]").within(city).geojson.save(
        "c:\\geodesk\\tests\\postboxes")
    routes = features("r[route=bicycle]").intersects(city).geojson.save(
        "c:\\geodesk\\tests\\routes")
    
def notest_wkt(features):
    for f in features[:100]:
        print(f.wkt)
        
def check_format(feature):
    geojson = json.loads(str(feature.geojson))
    s1 = to_mercator(shapely.geometry.shape(geojson['geometry']))
    assert s1.equals(feature.shape)
    wkt = str(feature.wkt)
    s1 = to_mercator(shapely.wkt.loads(wkt))
    assert s1.equals(feature.shape)
    

def test_format(features):
    point = features("n").first
    print(point.shape)
    linestring = features("w").first
    area_way = features.ways("a").first
    area_relation_single_simple = None
    area_relation_single_holes = None
    area_relation_multi_simple = None
    area_relation_multi_holes = None
    area_types_found = 0
    for f in features.relations("a"):
        try:
            shape = f.shape
        except RuntimeError:
            print(f"Failed to get shape for {f}:")
            # print(f"    {f.wkt}")
            # print(f"    {f.geojson}")
        if isinstance(shape, shapely.geometry.MultiPolygon):
            geom_count = len(shape.geoms)
            if geom_count == 1:
                print(f"{f} is a single-member MP")
            else:
                polygon = shape.geoms[0]
                if len(polygon.interiors) == 0:
                    if area_relation_multi_simple is None:
                        area_relation_multi_simple = f
                        area_types_found += 1
                else:
                    if area_relation_multi_holes is None:
                        area_relation_multi_holes = f
                        area_types_found += 1    
        else:  # single polygon               
            if len(shape.interiors) == 0:
                if area_relation_single_simple is None:
                    area_relation_single_simple = f
                    area_types_found += 1
            else:
                if area_relation_single_holes is None:
                    area_relation_single_holes = f
                    area_types_found += 1           
        if area_types_found == 4:
            # found one of each type
            break
    if area_types_found < 4:   
        raise RuntimeError(
            "Couldn't find a representative feature for " 
            "each kind of area-relation")
    
    check_format(point)
    check_format(linestring)
    check_format(area_way)
    check_format(area_relation_single_simple)
    check_format(area_relation_single_holes)
    check_format(area_relation_multi_simple)
    check_format(area_relation_multi_holes)
        
def notest_wkt_rounding(features):
    w = features("w[wikidata=Q7669413]").one
    str(w.wkt)
    