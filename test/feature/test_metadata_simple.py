# Copyright (c) 2025 Clarisma / GeoDesk contributors
# SPDX-License-Identifier: LGPL-3.0-only

def test_indexed_keys_simple(monaco):
    assert "highway" in monaco.indexed_keys