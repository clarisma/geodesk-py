# Copyright (c) 2024 Clarisma / GeoDesk contributors
# SPDX-License-Identifier: LGPL-3.0-only

def check_node(features, id):
    node = features.node(id)
    assert node is not None
    assert node.id == id

def check_way(features, id):
    way = features.way(id)
    assert way is not None
    assert way.id == id

def check_relation(features, id):
    rel = features.relation(id)
    assert rel is not None
    assert rel.id == id

def test_query_by_id(monaco):
    check_node(monaco, 4416197078)
    check_way(monaco, 626967072)
    check_relation(monaco, 2214022)
    assert monaco.node(222) is None

def notest_query_speed(monaco):
    count = 0
    for id in range(1,10000):
        if monaco.node(id) is not None:
            count += 1

def test_query_speed_manual(monaco):
    count = 0
    nodes = monaco.nodes
    for id in range(1,100000):
        if next((f for f in nodes if f.id == id), None) is not None:
            count += 1

