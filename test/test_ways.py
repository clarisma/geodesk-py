# Copyright (c) 2023 Clarisma / GeoDesk contributors
# SPDX-License-Identifier: LGPL-3.0-only

from geodesk import *

def test_ways(features):
    streets = features("w[highway=residential]")
    for street in streets[:1000]:
        bounds = Box()
        node_count = 0
        for node in street.nodes:
            bounds += node.centroid
            node_count += 1
        assert bounds == street.bounds
        assert node_count == street.nodes.count

def test_waynode_match(features):
    roads = features("w[highway]")
    total_bump_count = 0
    road_count = 0
    for road in roads:
        bumps = road.nodes("[traffic_calming=bump]")
        bump_count = bumps.count
        manual_bump_count = 0
        for node in road.nodes:
            if node.traffic_calming == "bump":
                manual_bump_count += 1
        assert bump_count == manual_bump_count
        total_bump_count += bump_count
        road_count += 1
    print(f"Found {total_bump_count} speed bumps in {road_count} roads.")

    speed_bumps = features("n")
    for street in roads[:1000]:
        bounds = Box()
        node_count = 0
        for node in street.nodes:
            bounds += node.centroid
            node_count += 1
        assert bounds == street.bounds
        assert node_count == street.nodes.count

