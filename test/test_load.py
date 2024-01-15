# Copyright (c) 2024 Clarisma / GeoDesk contributors
# SPDX-License-Identifier: LGPL-3.0-only

from geodesk import *

def notest_load():
    world = Features("c:\\geodesk\\tests\\w2.gol")
    world.load()