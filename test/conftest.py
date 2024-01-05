# Copyright (c) 2024 Clarisma / GeoDesk contributors
# SPDX-License-Identifier: LGPL-3.0-only

from geodesk import *
import pytest

@pytest.fixture(scope="session")
def features():
    f = Features('c:\\geodesk\\tests\\de.gol')
    # f = Features('c:\\geodesk\\tests\\w2.gol')
    yield f

@pytest.fixture(scope="session")
def monaco():
    yield Features("data/monaco")