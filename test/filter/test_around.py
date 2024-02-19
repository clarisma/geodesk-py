# Copyright (c) 2024 Clarisma / GeoDesk contributors
# SPDX-License-Identifier: LGPL-3.0-only

from geodesk import *

def test_around(features):
    munich = features("n[place=city][name:en=Munich]").one
    features("a[boundary]").around(munich, m=20).map(
        tooltip="{name}<br>{osm_type}/{id}", link="https://www.openstreetmap.org/{osm_type}/{id}").show()

def notest_around_lon_lat(features):
    m = Map(tooltip="{osm_type}/{id}", 
        link="https://www.openstreetmap.org/{osm_type}/{id}")
    m.add(Coordinate(lat=48.15247, lon=11.59344), color="red")
    # m.add(features.around(lat=48.15247, lon=11.59344, m=20))
    m.show()
