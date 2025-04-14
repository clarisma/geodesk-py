# Copyright (c) 2024 Clarisma / GeoDesk contributors
# SPDX-License-Identifier: LGPL-3.0-only

from shapely import *
from geodesk import *
import pytest

def test_coord_to_mercator():
    x,y = to_mercator(lat=30, lon=-178)
    c = Coordinate(x,y)
    x2 = to_mercator(lon=-178)
    y2 = to_mercator(lat=30)
    assert c == Coordinate(lat=30, lon=-178)
    assert c == Coordinate(x2,y2)
    assert round(c.lon, 7) == -178
    assert round(c.lat, 7) == 30

def test_coord_seq_to_mercator():
    seq1 = to_mercator(latlon=[[48.267,13.06],[49.53443,17.21321],[48.332,19.331],[50.6612,21.52312]])
    seq2 = to_mercator(latlon=[48.267,13.06,49.53443,17.21321,48.332,19.331,50.6612,21.52312])
    seq3 = to_mercator(lonlat=[13.06,48.267,17.21321,49.53443,19.331,48.332,21.52312,50.6612])
    seq4 = to_mercator(lonlat=[[13.06,48.267],[17.21321,49.53443],[19.331,48.332],[21.52312,50.6612]])
    seq5 = to_mercator(lonlat=[(13.06,48.267),(17.21321,49.53443),(19.331,48.332),(21.52312,50.6612)])
    seq6 = to_mercator(lonlat=((13.06,48.267),(17.21321,49.53443),(19.331,48.332),(21.52312,50.6612)))
    seq7 = to_mercator(lonlat=((13.06,48.267),Coordinate(lon=17.21321,lat=49.53443),(19.331,48.332),(21.52312,50.6612)))
    seq8 = to_mercator(13.06,48.267,17.21321,49.53443,19.331,48.332,21.52312,50.6612)
    assert seq1 == seq2
    assert seq1 == seq3
    assert seq1 == seq4
    assert seq1 == seq5
    assert seq1 == seq6
    assert seq1 == seq7
    

def test_length_to_mercator():
    d = to_mercator(km=20000, lat=0)
    assert to_mercator(meters=500, lat=60) == to_mercator(km=0.5, lat=60)
    with pytest.raises(TypeError):
        bad = to_mercator(feet=1000)


def test_invalid_to_mercator():
    bad = to_mercator([Coordinate(lon=-120,lat=90), Coordinate(lon=-180,lat=-90)])


def extract_coords(geom):
    """
    Recursively extract all coordinates from a Shapely geometry.
    """
    if geom.is_empty:
        return []
    if isinstance(geom, (Point, LineString, LinearRing)):
        return list(geom.coords)
    elif isinstance(geom, Polygon):
        # Extracting exterior and interior coordinates
        coords = list(geom.exterior.coords) 
        for interior in geom.interiors:
            coords.extend(interior.coords)
        return coords    
    elif isinstance(geom, (MultiPoint, MultiLineString, MultiPolygon, GeometryCollection)):
        # Recursively extract coordinates from each part
        coords = []
        for part in geom.geoms:
            coords.extend(extract_coords(part))
        return coords
    else:
        raise ValueError(f"Unsupported geometry type: {type(geom)}")

def test_convert_shape(features):
    f = features("a[boundary=administrative][admin_level=2][name:en=Germany]").one
    shape_wgs84 = from_mercator(f.shape)
    """
    coords = extract_coords(shape_wgs84)
    for c in coords:
        lon, lat = c
        assert lon >= -180 and lon <= 180
        assert lat >= -86 and lat <= 86
    """    
    shape_mercator = to_mercator(shape_wgs84)
    assert shape_mercator.equals(f.shape)
    # Make sure we're creating a copy, instead of transforming in-place
    assert shape_mercator is not shape_wgs84 

def test_convert_length():
    d = to_mercator(meters=100, lat=60)
    assert 100 == pytest.approx(from_mercator(d, unit="meters", lat=60))
    d = to_mercator(feet=1200, lat=38)
    assert 1200 == pytest.approx(from_mercator(d, unit="feet", lat=38))

    lat = -67.326
    c = lonlat(-112, lat)
    d = to_mercator(mi=1235.7, lat=lat)
    assert d == to_mercator(mi=1235.7, y=c.y)

def test_convert_length_bad_unit():
    with pytest.raises(TypeError):            
        d = to_mercator(turnips=1500, lat=48)
        
def test_convert_length_missing_lat():
    with pytest.raises(TypeError):       
        d = to_mercator(m=1500)

def test_convert_length_bad_lat():
    with pytest.raises(ValueError):       
        d = to_mercator(m=1500, lat=1000)
    
def notest_convert_shape_performance(features):
    f = features("a[boundary=administrative][admin_level=2][name:en=Germany]").one
    shape = f.shape
    for i in range(0,10000):
        from_mercator(shape)

def test_issue_64():
    # ls = LineString([(-1317617399, 515817921), (-1313128810, 520306510)])
    # ls2 = from_mercator(ls)
    mp = MultiPoint([(-1317617399, 515817921), (-1313128810, 520306510)])
    mp2 = from_mercator(mp)
    
