# Copyright (c) 2024 Clarisma / GeoDesk contributors
# SPDX-License-Identifier: LGPL-3.0-only

from geodesk import *
import json
import shapely.errors
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

def check_geojson_format(formatter, feature_shape):
    try:
        geojson = json.loads(str(formatter))
        s1 = to_mercator(shapely.geometry.shape(geojson['geometry']))
        assert s1.equals(feature_shape)
    except shapely.errors.GEOSException as ex: 
        print(f"GEOSException for {feature}: {ex}")
        formatter.save("c:\\geodesk\\debug\\bad-shape")
        assert False

def check_wkt_format(formatter, feature_shape):
    try:    
        s1 = to_mercator(shapely.wkt.loads(str(formatter)))
        assert s1.equals(feature_shape)
    except shapely.errors.GEOSException as ex: 
        print(f"GEOSException for {feature}: {ex}")
        formatter.save("c:\\geodesk\\debug\\bad-shape")
        assert False
        
def check_format(feature):
    try:    
        feature_shape = feature.shape
    except shapely.errors.GEOSException as ex: 
        print(f"GEOSException for {feature}: {ex}")
        feature.wkt.save("c:\\geodesk\\debug\\bad-shape")
        assert False
    check_geojson_format(feature.geojson(pretty=True), feature_shape)
    check_geojson_format(feature.geojson(pretty=False), feature_shape)
    check_wkt_format(feature.wkt(pretty=True), feature_shape)
    check_wkt_format(feature.wkt(pretty=False), feature_shape)
    
def test_test_basic_shapes(features):
    point = features("n").first
    linestring = features("w").first
    check_format(point)
    check_format(linestring)
    
def test_relations(features):
    relation = features("r[type=restriction]").first
    super_relation = features("r[type=superroute]").first
    check_format(relation)
    check_format(super_relation)

def test_area_features(features):
    area_relation_single_simple = None
    area_relation_single_holes = None
    area_relation_multi_simple = None
    area_relation_multi_holes = None
    area_types_found = 0
    for f in features.relations("a[landuse]"):
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
    
    check_format(area_relation_single_simple)
    check_format(area_relation_single_holes)
    check_format(area_relation_multi_simple)
    check_format(area_relation_multi_holes)
    
def notest_empty_features(features):
    check_format(features.nodes.relations)
               
def test_anon_nodes(features):
    w = features("w[highway]").first
    # TODO: need a proper test 
    nodes = w.nodes
    assert nodes.wkt is not None
    assert nodes.geojson is not None

def test_geojsonl(features):
    geojsonl = str(features("w[highway]").geojsonl(limit=10))
    print(geojsonl)
    # geojsonl_lines = geojsonl.strip().split('\n')

def test_maproulette(features):
    suspects = features("na[amenity=waste_basket,toilets,bench][name]")
    with open('c:\\geodesk\\tests\\challenge.geojsonl', 'w', encoding='utf-8') as file:
        for feature in suspects:
            # Write each task as a FeatureCollection (which in this case
            # only contains a single suspect features)
            file.write('\x1E{"type":"FeatureCollection","features":[')
            file.write(str(feature.geojson(id=lambda f: f"{f.osm_type}/{f.id}")))
            file.write(']}\n')

def test_pretty_geojson_save(features):    
    features("a[leisure=park]").geojson(limit=20, pretty=True).save(
        "c:\\geodesk\\tests\\pretty.geojson")