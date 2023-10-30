<img src="https://docs.geodesk.com/img/github-header.png">

GeoDesk is a fast and storage-efficient geospatial database for OpenStreetMap data. 
Also available [for Java](http://www.github.com/clarisma/geodesk).

## Why GeoDesk?

- **Small storage footprint** &mdash; GeoDesk's GOL files are only 20% to 50% larger than the original OSM data in PBF format &mdash; that's less than a tenth of the storage consumed by a traditional SQL-based database.

- **Fast queries** &mdash; typically 50 times faster than SQL. 

- **Fast to get started** &mdash; Converting `.osm.pbf` data to a GOL is 20 times faster than an import into an SQL database. Alternatively, download pre-made data tiles for just the regions you need and automatically assemble them into a GOL.

- **Intuitive API** &mdash; No need for object-relational mapping; GeoDesk queries return Python objects. Quickly discover tags, way-nodes and relation members. Get a feature's geometry, measure its length/area. 
 
- **Proper handling of relations** &mdash; (Traditional geospatial databases deal with geometric shapes and require workarounds to support this unique and powerful aspect of OSM data.)

- **Seamless integration with Shapely** for advanced geometric operations, such as buffer, union, simplify, convex and concave hulls, Voronoi diagrams, and much more.

- **Modest hardware requirements** &mdash; If it can run 64-bit Python, it'll run GeoDesk.
 
## Get Started

### Requirements

- Python 3.7 or above
- Java 16 or above (for the GOL Tool)
 
### Download

```
pip install geodesk
```

### Create a GOL

Create a Geographic Object Library based on any `.osm.pbf` file, using the 
[GOL Tool](https://www.geodesk.com/download) (Requires Java 16+).

For example:

```
gol build switzerland switzerland-latest.osm.pbf
```

### Example Application

Find all the pubs in Zurich (Switzerland) and print their names:

```python
from geodesk import *

# Open switzerland.gol
features = Features("switzerland")      

# Get the feature that represents the area of the city of Zurich
zurich = features("a[boundary=adminstrative][admin_level=8][name:en=Zurich]").one

# Define a set that contains nodes and areas that are pubs
pubs = features("na[amenity=pub]")

# Iterate through the pubs that are contained in the area of Zurich
# and print their names
for pub in pubs.within(zurich):
    print(pub.name)        
```

### More Examples

Find all movie theaters within 500 meters from a given point:

```python
movieTheaters = features("na[amenity=cinema]").around(
    meters=500, lat=47.37, lon=8.54)
```

*Remember, OSM uses British English for its terminology.*

Discover the bus routes that traverse a given street:

```python
for route in street.parents("[route=bus]")):
    print(f"- {route.ref} from {route.from} to {route.to}")
```

Count the number of entrances of a building:

```python
numberOfEntrances = building.nodes("[entrance]").count
```

## Documentation

[GeoDesk Developer's Guide](https://docs.geodesk.com/python)

## Related Repositories

- [geodesk](http://www.github.com/clarisma/geodesk) &mdash; GeoDesk for Java
- [gol-tool](http://www.github.com/clarisma/gol-tool) &mdash; command-line utility for building, maintaining and querying GOL files
