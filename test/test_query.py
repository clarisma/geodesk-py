# Copyright (c) 2024 Clarisma / GeoDesk contributors
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

def query(features, s):
    print(f"Query: {s}")  
    features(s)
    # print(f"Query: {s}\n{features.explain(s)}")

def no_test_negated_query(features):
    query(features, "w[highway=residential][maxspeed>20][maxspeed=*mph]")
    """
    query(features, "w[highway][bridge]")
    query(features, "w[!highway][!bridge]")
    query(features, "w[highway=primary,secondary]")
    query(features, "w[highway!=primary,secondary]")
    query(features, "w[highway][highway!=secondary,primary]")
    query(features, "w[highway][highway!=primary,secondary,tertiary,service,residential,path,footway]")
    """
    
# TODO: This test may fail if there are misspelled values of "highway"
# keys in the features dataset (e.g. "highway=sceondary")    
def test_issue_53(features):
    c1 = features("w[highway=*ary,residential]").count
    c2 = features("w[highway=primary,secondary,tertiary,residential]").count
    assert(c1 > 100)
    assert(c2 > 100)
    assert(c1 == c2)
    c1 = features("w[highway!=*ary,residential]").count
    c2 = features("w[highway!=primary,secondary,tertiary,residential]").count
    assert(c1 > 100)
    assert(c2 > 100)
    assert(c1 == c2)
    c1 = features("w[highway][highway!=*ary,residential]").count
    c2 = features("w[highway][highway!=primary,secondary,tertiary,residential]").count
    assert(c1 > 100)
    assert(c2 > 100)
    assert(c1 == c2)
    

    