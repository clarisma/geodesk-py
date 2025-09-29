# Copyright (c) 2024 Clarisma / GeoDesk contributors
# SPDX-License-Identifier: LGPL-3.0-only

from geodesk2 import *
import pytest

@pytest.fixture(scope="session")
def features():
    f = Features('d:\\geodesk\\tests\\de.gol')
    # f = Features('c:\\geodesk\\tests\\de3.gol')
    # f = Features('c:\\geodesk\\tests\\monaco.gol')
    yield f

@pytest.fixture(scope="session")
def monaco():
    yield Features("data/monaco")