# Copyright (c) 2024 Clarisma / GeoDesk contributors
# SPDX-License-Identifier: LGPL-3.0-only

from shapely import *
from geodesk import *
from itertools import pairwise
import pytest
import shapely
import geodesk
import units

def check_length(g):
    l = geodesk.length(g)
    for unit, factor in units.LENGTH_UNITS:
        # print(f"Testing distance in {unit} (factor = {factor})")
        l_unit = geodesk.length(g, unit)
        assert l_unit == l * factor
    return l

def test_length(monaco):
    street = monaco.way(166009792)    # Blvd Princesse Charlotte
    l = street.length
    l2 = check_length(street)
    l3 = check_length(street.shape)
    assert l == l2
    assert l == pytest.approx(l3, abs=0.001)
        # street.length and street.shape.length can differ slightly,
        # as street.length performs per-segment Mercator scaling
        # for greater accuracy (which may be overkill); for
        # length of a LineString, we just use scaling based on
        # the average Y of the bbox
        # (difference is less than a millimeter)


