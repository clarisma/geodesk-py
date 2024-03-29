# Copyright (c) 2024 Clarisma / GeoDesk contributors
# SPDX-License-Identifier: LGPL-3.0-only

def notest_nodes(features):
    streets=features("w[highway=service]")
    barriers=features("[barrier=lift_gate]")
    for street in streets:
        barrier_nodes = barriers.nodes_of(street)
        if barrier_nodes:
            print(f"{street} has {barrier_nodes.count} barriers")
            

def test_relation_nodes(features):
    rel = features.relations[0]
    assert not rel.nodes
    assert not features.nodes_of(rel)
