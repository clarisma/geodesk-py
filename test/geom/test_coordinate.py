# Copyright (c) 2024 Clarisma / GeoDesk contributors
# SPDX-License-Identifier: LGPL-3.0-only

from geodesk import *
import pytest

MIN_LAT = -85.0511288
MAX_LAT = 85.0511287

def test_init():
    c = Coordinate(100, 300)
    assert c.x == 100
    assert c.y == 300
    assert c[0] == 100
    assert c[1] == 300
    
    assert c == (100, 300)
    assert c == [100, 300]
    assert c != (100)
    assert c != [100]
    assert c != (100, 300, 2000)
    assert c != [100, 300, 2000]
    
    c2 = Coordinate(x=100, y=300)
    assert c == c2
    assert c2 == c
   
    c3 = Coordinate()
    assert c3.x == 0 and c3.y == 0
    assert c3 == (0,0)
    assert c3 != c

def test_init_with_wrong_args():
    with pytest.raises(ValueError) as ex_info:            
        bad = Coordinate(lon=190, lat=-100)
    assert "range" in str(ex_info.value)
    with pytest.raises(TypeError):            
        bad = Coordinate(set())
    with pytest.raises(TypeError):            
        bad = Coordinate(x="bad")
    
def test_init_clamped_range():
    c = Coordinate(lon=120, lat=90)
    assert c.lat == MAX_LAT
    c = Coordinate(lon=120, lat=-90)
    assert c.lat == MIN_LAT
    
def test_as_tuple():
    c = Coordinate(7000, 4000)        
    x,y = c
    assert x == c.x
    assert y == c.y

def test_lonlat():
    a = Coordinate(lon=-80, lat=30)
    b = lonlat(-80, 30)
    c = latlon(30, -80)
    assert a == b
    assert a == c

def test_lonlat_errors():
    with pytest.raises(ValueError):            
        bad = lonlat(-130, 999)
    with pytest.raises(TypeError):            
        bad = latlon("apple", "banana")
    with pytest.raises(TypeError):            
        bad = latlon(12)

def check_rounded(lon, lat):
    c = lonlat(lon, lat)
    assert c.lon == lon
    assert c.lat == lat

def test_rounded():
    check_rounded(-176.4321, 68.3921)
    minlat = -85.0511288 # Coordinate(0,-2147483648).lat
    maxlat = 85.0511287 # Coordinate(0,2147483647).lat (must be int_max-1)
    # print(f"minlat = {minlat}")
    # print(f"maxlat = {maxlat}")
    check_rounded(-180,minlat)
    check_rounded(180,maxlat)
    check_rounded(0,0)
    
def test_multi_lonlat():
    c0 = Coordinate(lon=120, lat=-63.5)
    c1 = Coordinate(lon=-166.4, lat=39)
    c2 = Coordinate(lon=-180, lat=-85.05)
    coords = [c0,c1]
    coords2 = [c0,c1,c2]
    l1 = lonlat(120, -63.5, -166.4, 39)
    assert l1 == coords
    l2 = lonlat((120, -63.5), (-166.4, 39))
    assert l2 == coords
    l3 = lonlat([120, -63.5], [-166.4, 39])
    assert l3 == coords
    l4 = lonlat([120, -63.5, -166.4, 39])
    assert l4 == coords
    l5 = lonlat(120, -63.5, -166.4, 39, -180, -85.05)
    assert l5 == coords2
    l6 = lonlat((120, -63.5), (-166.4, 39), (-180, -85.05))
    assert l6 == coords2
    l7 = lonlat([(120, -63.5), (-166.4, 39), (-180, -85.05)])
    assert l7 == coords2
    
def test_bad_multi_lonlat():
    # Must not mix flat and tupled coords
    with pytest.raises(TypeError):            
        bad = latlon(120, -63.5, (-166.4, 39))
    # Flat coords must appear in pairs
    with pytest.raises(TypeError):            
        bad = latlon(120, -63.5, -166.4)

def test_lonlat_range():
    c = lonlat(-180, 90)
    assert c == lonlat(-180, MAX_LAT)
    c = lonlat(180, -90)
    assert c == lonlat(180, MIN_LAT)
    with pytest.raises(ValueError):            
        bad = latlon(-180, 100)
    with pytest.raises(ValueError):            
        bad = latlon(-900.99, 50)
    with pytest.raises(ValueError):            
        bad = latlon(120.4, -90.1)
    with pytest.raises(ValueError):            
        bad = latlon(180.1, 90)
    
