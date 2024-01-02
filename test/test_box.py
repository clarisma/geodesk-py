# Copyright (c) 2024 Clarisma / GeoDesk contributors
# SPDX-License-Identifier: LGPL-3.0-only

from shapely import *
from geodesk import *
import pytest

INT_MIN = -2147483648
INT_MAX =  2147483647

def assert_coord_equal(actual, expected):
    assert abs(actual - expected) < 1e-7  # assuming 1e-7 as the tolerance, adjust as necessary

def test_init():
    box = Box(100,200,300,400)
    assert box.left == 100
    assert box.bottom == 200
    assert box.right == 300
    assert box.top == 400
    assert box.minx == 100
    assert box.miny == 200
    assert box.maxx == 300
    assert box.maxy == 400

    box = Box(lon=82, lat=5)
    assert_coord_equal(box.west, 82)
    assert_coord_equal(box.south, 5)
    assert_coord_equal(box.east, 82)
    assert_coord_equal(box.north, 5)
    assert_coord_equal(box.w, 82)
    assert_coord_equal(box.s, 5)
    assert_coord_equal(box.e, 82)
    assert_coord_equal(box.n, 5)
    assert_coord_equal(box.minlon, 82)
    assert_coord_equal(box.minlat, 5)
    assert_coord_equal(box.maxlon, 82)
    assert_coord_equal(box.maxlat, 5)

def test_init_partial():
    box = Box(north=30)
    print(box.maxy)
    print(box.maxlat)
    assert box.minx == INT_MIN
    assert box.maxx == INT_MAX
    assert box.miny == INT_MIN
    assert round(box.maxlat,7) == 30
    assert round(box.minlon,7) == -180
    assert round(box.maxlon,7) == 180


def test_init_world():
    box = Box(...)
    assert box.minx == INT_MIN
    assert box.miny == INT_MIN
    assert box.maxx == INT_MAX
    assert box.maxy == INT_MAX

def test_init_with_wrong_args():
    with pytest.raises(TypeError):
        bad = Box(lon="A")
    with pytest.raises(ValueError):
        bad = Box(lon=-180.1)
    with pytest.raises(ValueError):
        bad = Box(lat=91)


def test_contains():
    box = Box(100,200,300,400)
    box2 = Box(120,230,280,390)
    assert (150,250) in box
    assert (50,250) not in box
    assert box2 in box
    assert box not in box2
    assert [(100,400),(300,200)] in box
    assert [100,400,300,200,150,250] in box
    assert ((99,400),(300,200)) not in box

    assert Coordinate(150,250) in box
    assert [Coordinate(100,400),Coordinate(300,200)] in box

def test_intersection():
    a = Box(-300,-200,300,400)
    b = Box(100, -50, 700, 600)
    c = Box(100,-50,300,400)
    d = Box(100, 500, 200, 600)
    assert a & b == c
    assert a & c
    assert b & c
    assert not a & d

def test_as_tuple():
    box = Box(3000,2000,5000,4000)
    x1,y1,x2,y2 = box
    assert x1 == box.left
    assert y1 == box.bottom 
    assert x2 == box.right
    assert y2 == box.top 

def test_from_shapely_envelope():
    point = Point(500,600)
    assert Box(point) == Box(500,600)
    line = LineString([(200, 100), (200, 400), (300, 400)])
    assert Box(line) == Box(200,100,300,400)
    
def test_expand():
    box = Box()
    box += Coordinate(300, 400)
    assert box == Box(300, 400)
    box += Coordinate(-100, 400)
    assert box == Box(-100,400,300,400)
    box += Coordinate(700, -200)
    assert box == Box(-100,-200,700,400)
    
def test_buffer():
    box = Box(100, 200, 300, 400)
    box2 = Box(box)
    box.buffer(50)
    assert box == Box(50, 150, 350, 450)
    box.buffer(-50)
    assert box == box2
    box.buffer(meters=500)
    d = to_mercator(meters=500, y=box2.centroid.y)
    box.buffer(-d)
    assert box == box2
    box.buffer(-100)
    assert box == Box(200,300,200,300)
    box.buffer(-1)
    assert box == Box()

def test_buffer_empty():
    assert Box().buffer(4000) == Box()
    assert Box().buffer(-12.123) == Box()






