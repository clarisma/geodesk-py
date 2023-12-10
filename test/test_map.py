# Copyright (c) 2023 Clarisma / GeoDesk contributors
# SPDX-License-Identifier: LGPL-3.0-only

from geodesk import *
import pytest

def notest_map3(features):
    # Map(features("a[boundary=administrative][admin_level=4][name:en=Bavaria]"),
    Map(features("a[boundary=administrative][admin_level=4]"),
        tooltip="{type}/{id}: <b>{name}</b>",
        link="https://www.openstreetmap.org/relation/{id}",
        color="red").show()

def notest_map(features):
    all = features("a[landuse=forest]")
    invalid = []
    for f in all:
        try:
            shape = f.shape
        except Exception as ex:
            print (f"{f}: {ex} during shape creation")
            invalid.append(f)
            if len(invalid) > 50:
                break
    Map(invalid,
        tooltip="relation/{id}",
        link="https://www.openstreetmap.org/relation/{id}",
        color="red").show()
    
def notest_map2(features):
    map = Map()
    sushi = features("na[amenity=restaurant][cuisine=sushi,japanese]")
    map.add(sushi, tooltip="{type}/{id}: {name}<br>{lat}, {lon}", 
        link="https://www.openstreetmap.org/node/{id}",
        color="red")
    map.show()

def notest_map(features):
    map = Map();
    for f in features("r[route]")[:200]:
        map.add(f)
        map.add(f.centroid, color="red")
    map.show()

def notest_map2(features):
    map = Map(tooltip="{name}", weight=2, stroke=True)
    state = features("a[boundary=administrative][admin_level=4][name:en=Bavaria]").one
    counties = features("a[boundary=administrative][admin_level=6]")
    map.add(state, color="red", fillColor="yellow") # , fill="False")
    for county in counties.within(state):
        map.add(county, color="blue")
        # print(county.name)
    # buffered = state.shape.buffer(to_mercator(meters=500, lat=state.centroid.lat))
    # map.add(buffered, color="orange", fill=False)
    map.show()
    
    print(map.basemap)
    map.tooltip = None    
    print(map.tooltip)
    
    counties.within(state).map.show()    

def notest_map():
    world = Features("c:\\geodesk\\tests\\world.gol")
    italy = world("a[boundary=administrative][admin_level=2][name:en=Italy]").one
    counties = world("a[boundary=administrative][admin_level=6]")
    counties.within(italy).map(tooltip="{name} ({short_name})").show()

def notest_map_french_departments():
    world = Features("c:\\geodesk\\tests\\world.gol")
    france = world("a[boundary=administrative][admin_level=2][name=France]").one
    deps = world("a[boundary=administrative][admin_level=6]").within(france)
    deps.map(tooltip="{name}").add(Box(minlon=0, maxlon=0, minlat=-85,maxlat=85),
        color="red").show()
    
def notest_map_french_departments():
    world = Features("c:\\geodesk\\tests\\world.gol")
    france = world("a[boundary=administrative][admin_level=2][name=France]").one
    xxx = world("a[boundary=administrative][admin_level=6][name=Manche]").within(france).one
    m = Map()
    yyy = [ w for w in xxx.members.ways if w.id == 48728083]
    m.add(yyy, tooltip="way/{id}", color="orange")
    m.add(Box(minlon=0, maxlon=0, minlat=-85,maxlat=85), color="red")
    m.show()

def test_basemap_performance():
    m1 = Map(basemap="https://{s}.tile.openstreetmap.de/tiles/osmde/{z}/{x}/{y}.png")
    m1.show()
    m2 = Map(basemap="https://tile.openstreetmap.org/{z}/{x}/{y}.png")
    m2.show()
    