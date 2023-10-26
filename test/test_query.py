# Copyright (c) 2023 Clarisma / GeoDesk contributors
# SPDX-License-Identifier: LGPL-3.0-only

from geodesk import *

def get_monte_carlo(monaco):
    return monaco(
        "a[boundary=administrative][admin_level=10][name='Monte-Carlo']").one

def test_combine_queries(monaco):
    for b in monaco("a[boundary=administrative]"):
        print(f"{b.admin_level}: {b.name}")
        
    monte_carlo = get_monte_carlo(monaco)
    in_mc = monaco.within(monte_carlo)
    streets = monaco("w[highway=residential]")
    mc_streets = in_mc & streets
    mc_streets2 = streets & in_mc
    
    set_streets = set(streets)
    set_in_mc = set(in_mc)
    set_streets_in_mc = set_streets & set_in_mc
    assert mc_streets.count == len(set_streets_in_mc)
    assert mc_streets2.count == len(set_streets_in_mc)
    assert set_streets_in_mc == set(mc_streets)
    assert set_streets_in_mc == set(mc_streets2)
    
def no_test_filter_nodes(monaco):
    monte_carlo = get_monte_carlo(monaco)
    streets = monaco("w[highway=residential]")
    map = monte_carlo.map.add(streets, color="orange")
    for street in streets:
        map.add(street.nodes.within(monte_carlo), color="red")
        # map.add(street.nodes, color="red")
        # for node in street:
        #    map.add(node, color="red")
    map.show()    
    
def no_test_filter_members(monaco):
    monte_carlo = get_monte_carlo(monaco)
    routes = monaco("r[route=bus]")
    map = monte_carlo.map(leaflet_version="1.9.4").add(routes, color="orange", weight=6,
        tooltip="{osm_type}/{id}")
    for route in routes:
        map.add(route.members.within(monte_carlo), color="red")
    map.show()    