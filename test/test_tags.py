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

def test_tag_count(features):
    for street in features("w[highway=primary]"):
        # print(f"{street}: {street.tags}")
        count = len(street.tags)
        my_count = 0
        for k,v in street.tags:
            # print(f"{k} = {v}")
            my_count += 1
        assert count == my_count    
