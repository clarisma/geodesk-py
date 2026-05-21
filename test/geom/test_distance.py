# Copyright (c) 2024 Clarisma / GeoDesk contributors
# SPDX-License-Identifier: LGPL-3.0-only

from shapely import *
from geodesk import *
from itertools import pairwise
import pytest
import shapely
import geodesk

def test_way_distance_vs_length(monaco):
    for street in monaco("w[highway=primary]"):
        l = street.length
        nodes = list(street.nodes)
        d = 0
        for n1, n2 in pairwise(nodes):
            d += distance(n1, n2)
        assert d == pytest.approx(l)

def test_feature_distances(monaco):
    park = monaco.way(157719659)    # Petite Afrique
    park_shape = park.shape
    for street in monaco("w[highway]"):
        d1 = geodesk.distance(park, street)
        street_shape = street.shape
        d2 = geodesk.distance(park_shape, street_shape)
        d_shapely = from_mercator(park_shape.distance(street_shape), y=park.y)
        assert d1 == pytest.approx(d_shapely, abs=0.5)
        assert d2 == pytest.approx(d_shapely, abs=0.5)
        assert d1 == pytest.approx(d2)

def test_coordinate_distance_method():
    c1 = lonlat(8, 46)
    c2 = lonlat(-120, -36)
    d1 = distance(c1, c2)
    d2 = c1.distance(c2)
    d3 = c2.distance(c1)
    assert d1 == d2
    assert d1 == d3

