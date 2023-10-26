# Copyright (c) 2023 Clarisma / GeoDesk contributors
# SPDX-License-Identifier: LGPL-3.0-only

from geodesk import *

def test_ways(features):
    places = features("n[place][population]")
    for place in places[:100]:
        tags = place.tags
        # print(tags)
        tags_str = str(tags)
        assert '"place"' in tags_str
        assert '"population"' in tags_str
