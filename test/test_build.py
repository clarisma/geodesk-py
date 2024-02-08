from geodesk import *

def test_build():
    print("\n\n")
    test_path = "c:\\geodesk\\tests\\"
    source_path = "e:\\geodesk\\mapdata\\"
    world = Features(test_path + "de3", 
        source=source_path + "de-2021-01-29.osm.pbf",
        # source = "c:\\geodesk\\mapdata\\planet-2023-02-19.osm.pbf",
        # source = "e:\\geodesk\\mapdata\\planet-2023-10-07.osm.pbf",
        zoom_levels = [2,4,6,8,10,12],
        max_tiles = 64000,
        min_tile_density=25000,
        indexed_keys= [
            "place",
	        "highway", 
            "railway", 
            "aeroway", 
            "aerialway", 
            "tourism", 
            "amenity", 
            "shop", 
            "craft", 
            "power", 
            "industrial", 
            "man_made", 
            "leisure", 
            "landuse", 
            "waterway", 
            ("natural", "geological"), 
            "military", 
            "historic", 
            "healthcare", 
            "office", 
            "emergency", 
            "building", 
            "boundary", 
            "building:part", 
            ("telecom", "communication"), 
            "route",
        ],
        excluded_keys = [ "source" ]
	    )
    