# Copyright (c) 2024 Clarisma / GeoDesk contributors
# SPDX-License-Identifier: LGPL-3.0-only

from geodesk import *
import pytest
import sys

def test_query_parser(features):
    # print(features("w[highway=residential][name=A*][maxspeed<50]").count)
    # print(features("a[leisure=pitch][sport=soccer]").count)
    # print(features("a[leisure=pitch,park,playground]").count)
    # print(features("a[leisure]").count)
    # print(features("n[place=town][population>50000]").count)
    # print(features("w[highway=primary][maxspeed>90]").count)
    print(features("na[amenity=post_box][operator='D*']").count)
    # print(features("a[name='Kemal-Altun-Platz']").count)
    # print(features("a[name='Kem*']").count)
    # features("a[leisure=pitch][sport=soccer]")

    """
    features('na[name="*berg"]')
    features("w[monkey123!=banana123]");
    features("a[leisure=pitch][sport=soccer]")
    features("w[highway!=motorway,motorway_link]")
    features("w[maxspeed<50]")
    """
    """
    features("w[maxspeed=a,b,c][maxspeed>30][maxspeed=d,e,f]");
    features("w[highway][!highway]");
    features("w[highway][bridge][bridge!=aqueduct,boardwalk]");
    features("w[highway][bridge!=aqueduct,boardwalk][bridge]");
    features("w[name=BananaBananaXYZ]")
    features("w[highway!=motorway,motorway_link]");
    features("w[highway!=motorway]")
    features('na[name="*berg"]')
    features("w[highway!=motorway,motorway_link][!bridge]");
    features("w[maxspeed>30][maxspeed<90][maxspeed=*mph]");
    features("w[highway=residential,cycleway,footway,path,living_street]");
    """     
    
def notest_extreme(features):
    all = list(features)
    print(f"Count of all features:  {len(all)}")
    print(f"Size of list in bytes:  {sys.getsizeof(all)}")
    print(f"Size of single feature: {sys.getsizeof(all[0])}")
    
def notest_extreme_count(features):
    print(f"Count of all features: {features.count}")

def test_sizes(features):
    print(f"Size of Features object:   {sys.getsizeof(features)}")
    print(f"Size of Features iterator: {sys.getsizeof(iter(features))}")

def test_types(features):
    nodes = features("n[emergency=fire_hydrant]")
    for node in nodes:
        assert node.is_node
        assert node.osm_type == "node"

def test_areas(features):
    areas = features("a[landuse=retail]")
    for area in areas:
        assert area.is_area
        assert area.osm_type == "way" or area.osm_type == "relation"

def test_direct_instantiation():
    with pytest.raises(TypeError) as ex_info:      
        bad = Feature()
    assert "cannot create" in str(ex_info.value)
    
def test_count(features):
    parks = features('a[leisure=park]')
    count = parks.count
    assert count > 10000

    # len() is not allowed as substitute to .count
    # because it would cause query to execute twice when calling list()
    with pytest.raises(TypeError):      
        len(parks)

def test_query(features):
    parks = features('a[leisure=park]')
    for park in parks:
        assert park.is_area
        assert park.leisure == "park"
        assert park["leisure"] == "park"
        assert park.tags["leisure"] == "park"
        lat = park.lat
        assert lat >= -90 and lat <= 90
        lon = park.lon
        assert lon >= -180 and lon <= 180
        id = park.id
        assert id > 0 and id <= 16_000_000_000

def test_list(features):
    parks = features('a[leisure=park]')
    count = parks.count
    assert count > 0
    park_list = list(parks)
    assert len(park_list) == count
    
def test_bool(features):
    assert features('a[leisure=park]')
    assert not features('a[leisure=highway]')

def test_city_names(features):
    count = 0
    name_len = 0
    for city in features("na[place=city][name]"):
        count += 1
        name_len += len(city.name)
    name_len /= count
    assert name_len > 4 and name_len < 20

def test_members(features):
    routes = features('r[route=bicycle]')
    count = routes.count
    my_count = 0
    member_count = 0
    my_member_count = 0
    for rel in routes:
        my_count += 1
        member_count += rel.members.count
        for member in rel:
            my_member_count += 1
        for member in rel.members("w[highway=cycleway]"):
            assert member.highway=="cycleway"
    assert count > 100
    assert count == my_count
    assert my_member_count >= count
    assert my_member_count == member_count
    print(f"Number of relation members: {my_member_count}")

def test_hash(features):
    my_set = set()
    fire_stations = features("na[amenity=fire_station]")
    for station in fire_stations:
        my_set.add(station)
    for station in fire_stations:
        assert station in my_set
    
def test_ways(features):
    streets = features("w[highway=residential]")
    list = streets[:100]
    assert len(list) == 100
    for street in list:
        assert street.is_way
        assert street.osm_type == "way"
        assert not street.is_area
        assert street["highway"] == "residential"
        assert street.highway == "residential"
        my_node_count = 0
        for node in street:
            assert node.is_node
            assert node.osm_type == "node"
            assert node.centroid in street.bounds
            my_node_count += 1
        assert my_node_count >= 2

def test_shape(features):
    streets = features("w[highway=residential]")
    for street in streets[:100]:
        shape = street.shape
        assert shape is not None
        # str(shape)  # crashes! (nullptr)
        # print(type(shape))

    parks = features("a[leisure=park]")
    for park in parks[:100]:
        shape = park.shape
        assert shape is not None
        # print(type(shape))

def test_first(features):
    no_features = features("w[tourism=motorway_link]")
    with pytest.raises(QueryError):
        bad = no_features[0]
    assert no_features.first is None

def test_typed(features):
    nodes = features.nodes[:100]
    for f in nodes:
        assert f.is_node
    ways = features.ways[:100]
    for f in ways:
        assert f.is_way
    rels = features.relations[:100]
    for f in rels:
        assert f.is_relation

def test_empty(features):
    empty = features("n[amenity=restaurant]").ways
    assert empty.count == 0
    assert not empty
    assert empty.first is None
    assert empty.nodes is empty
    
def test_attribute_assignment(features):
    f = features("n[amenity=restaurant]")[0]
    with pytest.raises(AttributeError) as ex_info:            
        f.amenity = "post_box"
    assert "read-only" in str(ex_info.value)
    with pytest.raises(AttributeError) as ex_info:            
        del f.amenity
    assert "read-only" in str(ex_info.value)
