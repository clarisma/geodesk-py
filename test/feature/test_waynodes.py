# Copyright (c) 2024 Clarisma / GeoDesk contributors
# SPDX-License-Identifier: LGPL-3.0-only

from geodesk import *

def test_waynodes(monaco_updatable):
    for w in monaco_updatable("a[leisure=park]"):
        print(f"{w}: {w.name}")
        for n in w.nodes:
            print(f"- {n.id}: {n.lat},{n.lon}")
