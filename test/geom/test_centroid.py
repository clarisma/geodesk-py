# Copyright (c) 2026 Clarisma / GeoDesk contributors
# SPDX-License-Identifier: LGPL-3.0-only

import math

def test_centroid(monaco):
    count = 0
    for f in monaco("a"):
        shape = f.shape
        if shape is None or shape.is_empty or shape.area == 0:
            continue    # degenerate area: no meaningful areal centroid
        c = f.centroid
        sc = shape.centroid
        dist = math.hypot(c.x - sc.x, c.y - sc.y)
        assert dist <= 1.0, (
            f"{f}: centroid ({c.x}, {c.y}) is {dist:.1f} units "
            f"from GEOS centroid ({sc.x:.1f}, {sc.y:.1f})")
        count += 1
    assert count > 0