# Copyright (c) 2024 Clarisma / GeoDesk contributors
# SPDX-License-Identifier: LGPL-3.0-only

from geodesk import *

# If all nodes of a candidate lie on the test geometry's boundary,
# we need to test if the centroid of the candidate lies inside test

def test_issue_57(features):
    river_area = features(Box(lat=54.116507, lon=-0.826169))("a[natural=water][water=river]").one
    islet = features(Box(lat=54.106396,lon=-0.830951))("a[place=islet]").one
    assert islet not in features.within(river_area)
