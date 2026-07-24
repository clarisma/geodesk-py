# Copyright (c) 2024 Clarisma / GeoDesk contributors
# SPDX-License-Identifier: LGPL-3.0-only

from geodesk import *

def test_around(features):
    munich = features("n[place=city][name:en=Munich]").one
    features("a[boundary]").around(munich, m=20).map(
        tooltip="{name}<br>{osm_type}/{id}", link="https://www.openstreetmap.org/{osm_type}/{id}").show()

def test_around_lon_lat(features):
    m = Map(tooltip="{osm_type}/{id}", 
        link="https://www.openstreetmap.org/{osm_type}/{id}")
    c = latlon(48.15247, 11.59344)
    d= 500
    m.add(features.nodes.around(c, m=d), color="green")
    m.add(features("a").max_area(300).around(c, m=d))
    m.add(c, color="red")
    m.show()
