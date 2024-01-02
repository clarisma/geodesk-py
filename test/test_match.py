# Copyright (c) 2024 Clarisma / GeoDesk contributors
# SPDX-License-Identifier: LGPL-3.0-only

def notest_parser_errors(monaco):
    monaco("a[boundary")    # deliberately omits closing ]   
    
def test_parse(features):
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
    