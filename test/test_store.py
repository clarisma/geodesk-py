# Copyright (c) 2024 Clarisma / GeoDesk contributors
# SPDX-License-Identifier: LGPL-3.0-only

from geodesk import *
import pytest

def test_store_not_found():
    with pytest.raises(FileNotFoundError):
        Features("does-not-exist")
    
def test_bad_store():
    with pytest.raises(RuntimeError):
        Features("data/not-a-gol.gol")

def test_open_multiple():
    a = Features("data/monaco.gol")
    b = Features("data/monaco.gol")
    c = Features("data/monaco.gol")
    # TODO: a, b and c will be different objects, but they should
    # have the same FeatureStore

def test_without_extension():
    Features("data/monaco")
    