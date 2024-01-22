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
    streets=features("w[highway=primary]")[:100]
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
            
def test_street_endpoints(features):
    """
    A face validity test that checks that the endpoints of 
    secodary streets have at least two streets as parents
    (If this test fails, could also imply bad OSM data --
    secondary streets are not expected to simply end)
    """
    highways=features("w[highway]")
    streets=features("w[highway=secondary]")[:100]
    for street in streets:
        print(f"Checking {street}: {street.name} ({street.nodes.count} nodes):")
        nodes = list(street.nodes)
        assert len(nodes) >= 2
        print(f"  Start node: {nodes[0]}")
        print(f"    End node: {nodes[-1]}")
        print(f"  End node has these {nodes[-1].parents.ways.count} parent ways:")
        for parent in nodes[-1].parents.ways:
            print(f"  - {parent} (street: {parent in highways})")
        print(f"    Of these, {nodes[-1].parents.ways('w[highway]').count} are streets")
        assert nodes[0].parents.count >= 2
        assert nodes[-1].parents.count >= 2
        assert nodes[0].parents("w[highway]").count >= 2
        assert nodes[-1].parents("w[highway]").count >= 2

def test_debug(features):
    streets=features("w[highway=secondary]")[:100]
    for street in streets:
        print(f"Checking {street}: {street.name} ({street.nodes.count} nodes):")
        nodes = list(street.nodes)
        endnode = nodes[-1]
        if endnode.id == 0:
            print(f"  Anonymous endnode, {len(endnode.tags)} tags")
        else:    
            print(f"  Endnode: {endnode}")
            print(f"  Parents:         {endnode.parents.count}")
            print(f"  Parent highways: {endnode.parents('w[highway]').count}")

