# Copyright (c) 2024 Clarisma / GeoDesk contributors
# SPDX-License-Identifier: LGPL-3.0-only

def notest_18(features):
    assert areas.members_of(nodes[0]).count == 0
    
    for area in areas.nodes_of(nodes[0]):
        print(area)
   
def notest_19(features):
    myisland = features("a[wikidata=Q28179207]") 
    list(myisland.first.nodes)
   
def no_test_19x(features):
    for area in features("a"):
        if area.id == 381321380:
            print(area)
            for n in area.nodes:
                if n.id != 0:
                    print(f"- {n}")


