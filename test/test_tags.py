# Copyright (c) 2024 Clarisma / GeoDesk contributors
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

def test_empty_tags(features):
    """
    Calling `tags` on an anonymous node must return an empty tag set 
    """
    for way in feature.ways:
        for node in way.nodes:
            if node.id == 0:
                # Found an empty node
                tags = node.tags
                assert len(tags) == 0
                assert tags["highway"] is None
                assert str("highway") == ""
                assert num("whatever") == 0
                assert node.highway is None
                break