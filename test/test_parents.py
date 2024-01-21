# Copyright (c) 2024 Clarisma / GeoDesk contributors
# SPDX-License-Identifier: LGPL-3.0-only


def test_parent_relations(features):
    """
    The member/parent relationship must be reciprocal.
    """
    route=features("r[route=bicycle][network=lcn]")[0]
    for member in route:
        assert member.parents
        assert member.parents.relations
        assert route in member.parents
        assert route in member.parents.relations
        for parent in member.parents.relations:
            assert member in parent.members

def test_nonmember(features):
    """
    Obtaining `parents` of a feature that is not a relation
    member must not lead to failure (e.g. illegal read on a 
    non-existent reltable)
    """
    # We test with parks, because they are virtually never a
    # relation member; we check several just in case
    count = 0
    for park in features("a[leisure=park]"):
        if not park.parents:
            count += 1
            if count == 20:
                break

def test_parent_ways(features):
    """
    The way-node/parent-way relationship must be reciprocal.
    """
    streets=features("w[highway=primary]")
    for street in streets:
        # print(f"Checking {street}:")
        for node in street:
            # print(f"  Checking {node}:")
            assert node.parents
            assert node.parents.ways
            assert street in node.parents
            assert street in node.parents.ways
            for parent in node.parents.ways:
                # print(f"    {node} must be in {parent}")
                assert node in parent.nodes
            
