# Copyright (c) 2024 Clarisma / GeoDesk contributors
# SPDX-License-Identifier: LGPL-3.0-only

import inspect
import sys
from geodesk import *

def italian_restaurant_count(world):
    return world("na[amenity=restaurant][cuisine=italian]").count

def waynode_count(world):
    count = 0
    for way in world.ways:
        count += way.nodes.count
    return count

def waynode_iter_count(world):
    count = 0
    for way in world.ways:
        for node in way.nodes:
            count += 1
    return count

def street_crossing_count(world):
    crossings = world("n[highway=crossing]")
    count = 0
    for street in world("w[highway]"):
        count += crossings.nodes_of(street).count
    return count

def street_crossing_iter_count(world):
    crossings = world("n[highway=crossing]")
    count = 0
    for street in world("w[highway]"):
        for node in crossings.nodes_of(street):
            count += 1
    return count

def street_crossing_in_count(world):
    crossings = world("n[highway=crossing]")
    count = 0
    for street in world("w[highway]"):
        for node in street:
            if node in crossings:
                count += 1
    return count

def member_count(world):
    count = 0
    for parent in world:
        count += parent.members.count
    return count

def member_iter_count(world):
    count = 0
    for parent in world:
        for child in parent.members:
            count += 1
    return count

def parent_count(world):
    count = 0
    for child in world:
        count += child.parents.count
    return count

def parents_of_count(world):
    count = 0
    for child in world:
        count += world.parents_of(child).count
    return count

def parent_relations_of_count(world):
    relations = world.relations
    count = 0
    for child in world:
        count += relations.parents_of(child).count
    return count

def parent_ways_of_count(world):
    ways = world.ways
    count = 0
    for child in world:
        count += ways.parents_of(child).count
    return count

def parent_iter_count(world):
    count = 0
    for child in world:
        for parent in child.parents:
            count += 1
    return count

def parent_relations_count(world):
    count = 0
    for child in world:
        count += child.parents.relations.count
    return count

def parent_ways_count(world):
    count = 0
    for child in world:
        count += child.parents.ways.count
    return count

def nonsense_parent_count(world):
    count = 0
    for child in world:
        count += child.parents.nodes.count
    return count

def nonsense_parents_of_count(world):
    nodes = world.nodes
    count = 0
    for child in world:
        count += nodes.parents_of(child).count
    return count

def waynode_parents_count(world):
    count = 0
    for way in world.ways:
        for node in way.nodes:
            count += node.parents.count
    return count

def waynode_parent_ways_count(world):
    count = 0
    for way in world.ways:
        for node in way.nodes:
            count += node.parents.ways.count
    return count

def tags_count(world):
    count = 0
    for f in world:
        count += len(f.tags)
    return count

def tags_iter_count(world):
    count = 0
    for f in world:
        for tag in f.tags:
            count += 1
    return count

def tags_key_len(world):
    total_len = 0
    for f in world:
        for k,v, in f.tags:
            total_len += len(k)
    return total_len

def tags_str_len(world):
    total_len = 0
    for f in world:
        for k,v, in f.tags:
            total_len += len(f.str(k))
    return total_len

def tags_int_sum(world):
    sum = 0
    for f in world:
        for k,v, in f.tags:
            sum += int(f.num(k))
    return sum & 0x7FFFFFFFFFFFFFFF  # must be int64

def xy_hash(world):
    hash = 0
    for f in world:
        hash ^= f.x
        hash ^= f.y
    return hash

def centroid_hash(world):
    hash = 0
    for f in world:
        c = f.centroid
        hash ^= c.x
        hash ^= c.y
    return hash

def lonlat_100nd_hash(world):
    hash = 0
    for f in world:
        hash ^= int(f.lon * 10000000)
        hash ^= int(f.lat * 10000000)
    return hash

def waynodes_lonlat_100nd_hash(world):
    hash = 0
    for way in world.ways:
        for node in way.nodes:
            hash ^= int(node.lon * 10000000)
            hash ^= int(node.lat * 10000000)
    return hash

def id_hash(world):
    hash = 0
    for f in world:
        hash ^= f.id
    return hash

def geojson_len(world):
    total_len = 0
    for f in world:
        total_len += len(str(f.geojson))
    return total_len

def helper_read_key_value_file(file_path):
    result = {}
    with open(file_path, 'r') as file:
        for line in file:
            line = line.strip()
            if not line:    # ignore empty lines
                continue
            key, value = line.split('=', 1)
            result[key] = int(value)
    return result

def helper_check_results(results, file_path):
    passed = True
    good = helper_read_key_value_file(file_path)
    performed = set()
    for name,res in results:
        performed.add(name)
        if name not in good:
            print(f"No last-known-good for {name}")
            passed = False
            continue
        good_res = good[name]
        if good[name] != good_res:
            print(f"{name} should be {good_res} instead of {res}")
            passed = False
    for name in good.keys():
        if name not in performed:
            print(f"Failed to perform {name}")
            passed = False
    return passed




def test_concur(features):
    # Always use monaco.gol for this test
    world = Features("data/monaco")
    functions = [ (name,obj) for name,obj in inspect.getmembers(sys.modules[__name__])
        if inspect.isfunction(obj)
            and not name.startswith('test_')
            and not name.startswith('helper_')]

    results = []
    for name, func in functions:
        res = func(world)
        print (f"{name}: {res}")
        results.append((name, res))

    """
    with open("c:\\geodesk\\tests\\monaco-current.txt", "w") as file:
        for name,res in results:
            file.write(f"{name}={res}\n")
    """

    assert helper_check_results(results, "data/monaco-good.txt")


