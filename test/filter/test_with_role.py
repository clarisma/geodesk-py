# Copyright (c) 2024 Clarisma / GeoDesk contributors
# SPDX-License-Identifier: LGPL-3.0-only

import pytest
from shapely import *
from geodesk import *

def test_train_stops(features):
    stops = features.with_role("stop")
    routes = features("r[route=train]")
    total_stop_count = 0
    for route in routes:
        manual_stop_list = [ m for m in route.members if m.role == "stop"]
        assert list(stops.members_of(route)) == manual_stop_list
        total_stop_count += len(manual_stop_list)
    assert total_stop_count > 0

def test_multi_roles(features):
    forward_backward = features.with_role("forward", "backward")
    routes = features("r[route=bus]")
    total_count = 0
    for route in routes:
        manual_list = [ m for m in route.members if m.role in ["forward", "backward"] ]
        assert list(forward_backward.members_of(route)) == manual_list
        total_count += len(manual_list)
    assert total_count > 0

def test_invalid_queries(features):
    with pytest.raises(TypeError):
        bad = features.with_role()
    with pytest.raises(TypeError):
        bad = features.with_role(bananas=12)
    with pytest.raises(TypeError):
        bad = features.with_role(["a","b","c"],bananas=12)
    with pytest.raises(TypeError):
        bad = features.with_role(1,2,3)
    with pytest.raises(TypeError):
        bad = features.with_role(["a","b"],["c","d"])

def test_with_role_and_min_area(features):
    MIN_AREA = 200
    platforms = features.min_area(MIN_AREA).with_role("platform")
    routes = features("r[route=train]")
    total_count = 0
    for route in routes:
        manual_list = [ m for m in route.members if m.role == "platform" and m.area >= MIN_AREA]
        assert list(platforms.members_of(route)) == manual_list
        total_count += len(manual_list)
    assert total_count > 0

def test_with_role_and_min_and_max_area(features):
    MIN_AREA = 200
    MAX_AREA = 250

    def accept(f):
        if not f.role == "platform":
            return False
        a = f.area
        return MIN_AREA <= a <= MAX_AREA

    platforms = features.min_area(MIN_AREA).max_area(MAX_AREA).with_role("platform")
    routes = features("r[route=train]")
    total_count = 0
    for route in routes:
        manual_list = [ m for m in route.members if accept(m)]
        assert list(platforms.members_of(route)) == manual_list
        total_count += len(manual_list)
    assert total_count > 0