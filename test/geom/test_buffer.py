# Copyright (c) 2026 Clarisma / GeoDesk contributors
# SPDX-License-Identifier: LGPL-3.0-only

from geodesk import *
import pytest
import shapely
import geodesk

def test_buffer(monaco):
    park = monaco.way(157719659)    # Petite Afrique
    park_shape = park.shape
    park_buffered = geodesk.buffer(park_shape, 50)
    d = to_mercator(meters=50, y=park.y)
    park_buffered2 = shapely.buffer(park_shape, d)
    park_buffered3 = park.buffer(50)
    assert park_buffered.area == pytest.approx(park_buffered2.area)
    assert park_buffered3.area == pytest.approx(park_buffered.area)

    node = park.nodes.first
    circle = geodesk.buffer(node, 100)
    scale = to_mercator(meters=1, y=node.y)
    assert circle.area == pytest.approx(31415.926535 * scale * scale, rel=0.1)

    box = park.bounds
    box_buffered_shape = geodesk.buffer(box, -5)
    box.buffer(meters=-5)
    box_buffered_shape2 = box.shape
    assert box_buffered_shape2.area == pytest.approx(box_buffered_shape.area, rel=0.01)
