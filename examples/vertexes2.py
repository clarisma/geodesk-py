import sys
from geodesk import *

object_keys = {
    "advertising", "aerialway", "aeroway", "amenity", "barrier", "boundary", "building", "building:part",
    "communication", "craft", "crossing", "emergency", "entrance", "exit", "geological", "hazard", "healthcare",
    "highway", "historic", "indoor", "industrial", "kerb", "landuse", "leisure", "man_made",
    "military", "natural", "obstacle", "office", "place", "power", "public_transport", 
    "railway", "road_marking", "roof:ridge", "route", "seamark:type", "shop", "surveillance", "telecom", "tourism", 
    "traffic_calming", "traffic_sign", "traffic_sign:forward", "traffic_sign:backward", 
    "waterway"
}

aux_keys = { "bdouble", "canoe", "crossing:markings", "depth", "ele", "ford", "kerb:height", "level", "network:type", "noexit",
            "tactile_paving", "whitewater"}

meta_keys = { "comment", "converted_by", "created_by", "description", "fixme", "FIXME", "note",
             "odbl", "source", "todo" }

group_prefixes = { "addr", "area", "TMC", "ele", "historic", "railway" }

lifecycle_prefixes = { "abandoned", "construction", "demolished", "disused", "planned", "proposed", "razed", "removed", "was" }


if len(sys.argv) < 2:
    print("Must specify a GOQL query")
    sys.exit()
world = Features("c:\\geodesk\\tests\\de3.gol")
nodes = world(sys.argv[1]).nodes
counts = {}

for node in nodes:
    for way in node.parents.ways:
        tag_count = 0
        tag = None
        aux_tag = None
        group_tag = None
        has_lifecycle = False
        has_meta = False
        for k,v in way.tags:
            tag_count += 1
            if k in object_keys:
                tag = f"{k}={v}"
                break
            elif k in aux_keys:
                aux_tag = k
            elif k in meta_keys:
                has_meta = True
            else:
                n = k.find(':')
                if n >= 0:
                    prefix = k[0:n]
                    if prefix in group_prefixes:
                        group_tag = prefix + ":*"
                    elif prefix in lifecycle_prefixes:
                        has_lifecycle = True
        if tag_count == 0:
            continue
        if not tag:
            if group_tag:
                tag = group_tag
            elif aux_tag:
                tag = aux_tag
            elif has_lifecycle:
                tag = "(lifecycle)"
            elif has_meta:
                tag = "(metadata)"
        if tag:
            if tag in counts:
                counts[tag] += 1
            else:
                counts[tag] = 1
        else:
            print(f"{way}: {way.tags}")

for tag, count in sorted(counts.items(), key=lambda item: item[1], reverse=True):
    print(f"{count} {tag}")