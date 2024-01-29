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
        assert features.parents_of(member).count == member.parents.count
        assert features.relations.parents_of(member).count == member.parents.relations.count
        

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
            # print(f"  parents_of(node): {list(features.parents_of(node))}")
            # print(f"  node.parents:     {list(node.parents)}")
            assert features.parents_of(node).count == node.parents.count
            assert features.ways.parents_of(node).count == node.parents.ways.count
            
def test_street_endpoints(features):
    """
    A face validity test that checks that the endpoints of 
    primary streets have at least two streets as parents
    (If this test fails, could also imply bad OSM data --
    secondary streets are not expected to simply end)
    TODO: ^^^ Not necessarily -- major street could end at
    a ferry terminal etc. / disabling for now
    """
    highways=features("w[highway]")
    streets=features("w[highway=primary]")[:100]
    for street in streets:
        # print(f"Checking {street}: {street.name} ({street.nodes.count} nodes):")
        nodes = list(street.nodes)
        assert len(nodes) >= 2
        # print(f"  Start node: {nodes[0]}")
        # print(f"    End node: {nodes[-1]}")
        # print(f"  End node has these {nodes[-1].parents.ways.count} parent ways:")
        # for parent in nodes[-1].parents.ways:
        #    print(f"  - {parent} (street: {parent in highways})")
        # print(f"    Of these, {nodes[-1].parents.ways('w[highway]').count} are streets")
        assert nodes[0].parents.count >= 2
        assert nodes[-1].parents.count >= 2
        assert nodes[0].parents("w[highway]").count >= 2
        assert nodes[-1].parents("w[highway]").count >= 2

def test_both_parent_types(features):
    """
    Ensures that node.parents works if node is part of ways as well
    as relations
    """
    test_done = False
    for way in features.ways:
        for node in way:
            rel_count = node.parents.relations.count
            way_count = node.parents.ways.count
            assert node.parents.count == rel_count + way_count
            if rel_count and way_count:
                print(f"Found {node} with both relations and ways as parent; test passed")
                test_done = True
                break
        if test_done:
            break


