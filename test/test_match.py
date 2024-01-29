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
    hotel_count = hotels.count
    restaurant_count = restaurants.count
    both_count = both.count
    assert both_count > 1           # May fail depending on data
    assert both_count < hotel_count
    assert both_count < restaurant_count
    assert both_count == both2.count
    print(f"{hotel_count} hotels")
    print(f"{restaurant_count} restaurants")
    print(f"{both_count} hotels that are also restaurants")
    
def test_issue_45(features):
    countries = features('ar[boundary=administrative][admin_level=2][type!=multilinestring]').relations
    countries('*[name:en=Germany]').one