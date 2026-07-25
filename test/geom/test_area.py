# Copyright (c) 2024 Clarisma / GeoDesk contributors
# SPDX-License-Identifier: LGPL-3.0-only

from shapely import *
from geodesk import *
from itertools import pairwise
import pytest
import shapely
import geodesk
import units

def check_area(g):
    a = geodesk.area(g)
    for unit, factor in units.LENGTH_UNITS:
        # print(f"Testing distance in {unit} (factor = {factor})")
        a_unit = geodesk.area(g, unit)
        assert a_unit == a * factor * factor
    return a

def test_area(monaco):
    park = monaco.way(157719659)    # Petite Afrique
    a = park.area
    a2 = check_area(park)
    a3 = check_area(park.shape)
    assert a == a2
    assert a == a3

    b = park.bounds.area
    b2 = check_area(park.bounds)
    b3 = check_area(park.bounds.shape)
    assert b == b2
    assert b == b3
    assert b >= a           # Area of bounds must be >= area of the feature



