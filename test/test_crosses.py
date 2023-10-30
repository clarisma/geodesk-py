# Copyright (c) 2023 Clarisma / GeoDesk contributors
# SPDX-License-Identifier: LGPL-3.0-only

from geodesk import *

def test_crosses(features):
    map = Map()
    bavaria = features("a[boundary=administrative][admin_level=4][name:en=Bavaria]").one
    danube = features("r[waterway=river][name:en=Danube]").one
    map.add(danube)
    rail_bridges = features("w[railway][bridge]")
    map.add(rail_bridges.crosses(danube)(bavaria), color="red")
    map.show()