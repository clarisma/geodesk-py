# Copyright (c) 2023 Clarisma / GeoDesk contributors
# SPDX-License-Identifier: LGPL-3.0-only

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
    
