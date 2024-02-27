# Copyright (c) 2024 Clarisma / GeoDesk contributors
# SPDX-License-Identifier: LGPL-3.0-only

def notest_parser_errors(monaco):
    monaco("a[boundary")    # deliberately omits closing ]   
    
def test_parse(features):
    features("ar[boundary=administrative][admin_level=2][type!=multilinestring][name:en=Germany]")
    features("n")
    # state = features("a[boundary=administrative][admin_level=4][name:en=Bavaria]")[0]
    # country = features("a[boundary=administrative][admin_level=2][name:en=Germany]")[0]
    
    # features("na[amenity=restaurant]")
    # features("w[highway=residential,primary,secondary]")

    # features("na[xxamenity!=restaurant][bananas=999]")        
    # features("na[amenity=restaurant][bananas=999], na[amenity=cafe][bananas=999]")        

    # features("w[highway != motorway, motorway_link, *track]")    # does not work 
    # features("na[amenity=restaurant], na[amenity=cafe]")    
    # features("w[highway][maxspeed<55][maxspeed=*mph]")    
    # features("w[highway][bridge]")    
    # features("w[!highway][!bridge]")   
    # features("w[highway != residential]")  
    # features("w[maxspeed>20][highway=residential,primary,secondary,*ary][maxspeed<90]")  
    
def test_combined(features):
    hotels = features("na[tourism=hotel]")
    restaurants = features("na[amenity=restaurant]")
    both = hotels & restaurants
    both2 = hotels("na[amenity=restaurant]")
    both_explicit = features("na[tourism=hotel][amenity=restaurant]")
    hotel_count = hotels.count
    restaurant_count = restaurants.count
    both_count = both.count
    assert both_count > 1           # May fail depending on data
    assert both_count < hotel_count
    assert both_count < restaurant_count
    assert both_count == both2.count
    assert both_count == both_explicit.count
    print(f"{hotel_count} hotels")
    print(f"{restaurant_count} restaurants")
    print(f"{both_count} hotels that are also restaurants")
    
def test_issue_45(features):
    countries = features('ar[boundary=administrative][admin_level=2][type!=multilinestring]').relations
    countries('*[name:en=Germany]').one
    
def test_negative(features):
    features("w[highway]")         
    features("w[highway=primary]") 
    features("w[!highway]")                 # ok
    features("w[highway != primary]")       # ok
    features("w[highway][name != 'Main Street']")  # ok
    features("w[!highway][name = 'Main Street']")  # ok 
    features("w[!highway][name != 'Apple Street']")    # ok 
    features("na[!tourism][!amenity]")  # ok
    features("na[tourism != hotel][amenity != restaurant]")  # ok
    features("na[tourism != hotel][amenity != restaurant]")  # ok
    features("w[highway != primary,secondary]")  # ok
    """
     0  NOT FIRST_GLOBAL_KEY highway (3) -> 12
     3  NOT LOAD_CODE -> 12
     5  EQ_CODE secondary (202) -> 11
     8  NOT EQ_CODE primary (301) -> 12
    11  RETURN FALSE
    12  RETURN TRUE
    """
    features("w[highway][highway != motorway]")  
    # ... produces bad opcodes:
    """
     0  NOT FIRST_GLOBAL_KEY highway (3) -> 12
     3  NOT LOAD_CODE -> 8     <-- !!! This should branch to 11 !!!
     5  EQ_CODE no (1) -> 12
     8  EQ_CODE motorway (579) -> 12
    11  RETURN TRUE
    12  RETURN FALSE
    """
    # features("w[highway != primary,secondary,1,2]") 
    # causes assertion failure
    # features("w[highway != motorway, monkey_bananas]")  
    # causes assertion failure
        

def test_negated_unary(features):    
    area = features("a[boundary=administrative][admin_level=9][name='Altstadt-Lehel']").one
    nodes = features.nodes(area)
    tourism = nodes("n[tourism]")
    tourism.map.show()
    not_tourism = nodes("n[!tourism]")
    not_tourism.map.show()
    for f in tourism:
        assert f not in not_tourism
    for f in not_tourism:
        assert f not in tourism
    assert nodes.count == tourism.count + not_tourism.count

def test_negated_unary2(features):
    way_count = features.ways.count
    highway_count = features.ways("[highway]").count
    not_highway_count = features.ways("[!highway]").count
    assert highway_count + not_highway_count == way_count

def test_negated_unary3(features):
    way_count = features.nodes.count
    highway_count = features.nodes("[highway]").count
    not_highway_count = features.nodes("[!highway]").count
    assert highway_count + not_highway_count == way_count
    