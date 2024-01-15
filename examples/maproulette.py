# Example MapRoulette Challenge: Find mundane features like garbage bins
# and toilets that have a "name" tag (Ordinarily, these features should
# not have names)
# See https://community.openstreetmap.org/t/105540

import geodesk

world = geodesk.Features('c:\\geodesk\\tests\\w2.gol')
suspects = world("na[amenity=waste_basket,toilets,bench][name]")
with open('c:\\geodesk\\tests\\challenge.geojsonl', 'w', encoding='utf-8') as file:
    for feature in suspects:
        # Write each task as a FeatureCollection (which in this case
        # only contains a single suspect features)
        # MapRoulette also wants lines to start with a record separater
        # (ASCII hex 1E), so we write it explicitly    
        file.write('\x1E{"type":"FeatureCollection","features":[')
        file.write(str(feature.geojson(id=lambda f: f"{f.osm_type}/{f.id}")))
        file.write(']}\n')
