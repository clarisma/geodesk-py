# Copyright (c) 2024 Clarisma / GeoDesk contributors
# SPDX-License-Identifier: LGPL-3.0-only

import geodesk

def test_dir(features):
    print(dir(geodesk.Features))
    print(dir(features))
    