# Copyright (c) 2024 Clarisma / GeoDesk contributors
# SPDX-License-Identifier: LGPL-3.0-only

from geodesk import *
import pytest
# from shapely.validation import explain_validity
import re

def report_feature(mapview, feature, shape):
    cause = explain_validity(shape)
    mapview.add(feature, tooltip=f"relation/{feature.id}: {cause}", 
        link=f"https://www.openstreetmap.org/relation/{feature.id}")
    match = re.search(r'\[(\d+) (\d+)\]', cause)
    if match:
        x, y = map(int, match.groups())
        mapview.add(Coordinate(x,y), color="red")
            

def notest_polygonizer(features):
    map = Map(
        tooltip="relation/{id}: {cause}",
        link="https://www.openstreetmap.org/relation/{id}",
        color="red")
    # all = features("a[landuse=forest]")
    all = features("a[landuse]")
    invalid_count = 0
    for f in all:
        # if f.id != 10978807:
        #    continue
        try:
            shape = f.shape
        except Exception as ex:
            cause = f"{f}: {ex} during shape creation"
            map.add(f)
            invalid_count += 1
        else:
            if shape.is_empty:
                cause = "empty" 
                map.add(f)
                invalid_count += 1
            elif not shape.is_valid:    
                # report_feature(map,f,shape)
                invalid_count += 1
        # if invalid_count > 500:
        #    break
    print(f"Found {invalid_count} invalid polygons")
    map.show()
    
def notest_map2(features):
    map = Map()
    sushi = features("na[amenity=restaurant][cuisine=sushi,japanese]")
    map.add(sushi, tooltip="{type}/{id}: {name}<br>{lat}, {lon}", 
        link="https://www.openstreetmap.org/node/{id}",
        color="red")
    map.show()