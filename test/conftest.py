# Copyright (c) 2023 Clarisma / GeoDesk contributors
# SPDX-License-Identifier: LGPL-3.0-only

from geodesk import *
import pytest

@pytest.fixture(scope="session")
def features():
    f = Features('c:\\geodesk\\tests\\de.gol')
    yield f

@pytest.fixture(scope="session")
def monaco():
    yield Features("data/monaco")