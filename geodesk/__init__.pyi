from os import waitid_result
from shapely import Geometry
from typing import Callable, Dict, Iterator, List, Sequence, Union

class Box:
    def __init__(self, /, minx: int, miny: int, maxx: int, maxy: int, *,
        minlon: float, minlat: float, maxlon: float, maxlat: float,
        left: int, right: int, top: int, bottom: int, 
        west: float, south: float, east: float, north: float,  
        w: float, s: float, e: float, n: float) -> None: ...
    minlon: float
    minlat: float
    maxlon: float
    maxlat: float
    minx: int
    miny: int
    maxx: int
    maxy: int
    left: int
    right: int
    top: int
    bottom: int
    west: float
    south: float
    east: float
    north: float
    w: float
    s: float
    e: float
    n: float
    centroid: 'Coordinate'
    shape: Geometry
    def buffer(self, *, meters: float, m: float, feet: float, ft: float, km: float, miles: float) -> 'Box': ...
    def __and__(self, other: 'Box') -> 'Box': ...   
    
class Coordinate:
    x: int
    y: int
    lon: float
    lat: float

class Feature:
    bounds: 'Box'
    geojson: 'Formatter'
    id: int
    is_area: bool
    is_node: bool
    is_relation: bool
    is_way: bool
    lat: float
    length: float
    lon: float
    map: 'Map'
    members: 'Features'
    nodes: 'Features'
    parents: 'Features'
    def num(self, key: str) -> float: ...
    osm_type: str
    role: str
    shape: Geometry
    def str(self, key: str) -> str: ...
    tags: 'Tags'
    wkt: 'Formatter'
    x: int
    y: int

class Features:
    def __init__(self, filename: str) -> None: ...
    area: float
    count: int
    first: 'Feature' | None
    geojson: 'Formatter'
    geojsonl: 'Formatter'
    indexed_keys: List[str]
    length: float
    list: List['Feature']
    map: 'Map'
    nodes: Features
    one: 'Feature'
    properties: Dict[str,str]
    relations: 'Features'
    revision: int
    shape: Geometry
    strings: List[str]
    tiles: List['Tile']
    timestamp: str
    ways: 'Features'
    wkt: 'Formatter'
    def around(self, geom: Union['Box', 'Coordinate', 'Feature', Geometry], *, meters: float, m: float, feet: float, ft: float, km: float, miles: float) -> 'Features': ...
    def connected_to(self, geom: Union['Box', 'Coordinate', 'Feature', Geometry]) -> 'Features': ...
    def contains(self, geom: Union['Box', 'Coordinate', 'Feature', Geometry]) -> 'Features': ...
    def contained_by(self, geom: Union['Box', 'Coordinate', 'Feature', Geometry]) -> 'Features': ...
    def crosses(self, geom: Union['Box', 'Coordinate', 'Feature', Geometry]) -> 'Features': ...
    def disjoint(self, geom: Union['Box', 'Coordinate', 'Feature', Geometry]) -> 'Features': ...
    def intersects(self, geom: Union['Box', 'Coordinate', 'Feature', Geometry]) -> 'Features': ...
    def members_of(self, feature: 'Feature') -> 'Features': ...
    def nodes_of(self, feature: 'Feature') -> 'Features': ...
    def overlaps(self, geom: Union['Box', 'Coordinate', 'Feature', Geometry]) -> 'Features': ...
    def parents_of(self, feature: 'Feature') -> 'Features': ...
    def touches(self, geom: Union['Box', 'Coordinate', 'Feature', Geometry]) -> 'Features': ...
    def within(self, geom: Union['Box', 'Coordinate', 'Feature', Geometry]) -> 'Features': ...
    def __and__(self, other: "Features") -> "Features": ...
    def __iter__(self) -> Iterator['Feature']: ...
    def __contains__(self, item: 'Feature') -> bool: ...
    
class Formatter:
    id: Union[str, Callable[['Feature'], Union['str',int]]]
    limit: int
    linewise: bool
    mercator: bool
    precision: int
    pretty: bool
    def save(self, filename: str) -> None: ...
    
class Map:
    def __init__(self, *,
        basemap :str, attribution: str, leaflet_version: str, leaflet_url: str,
        leaflet_stylesheet_url: str, min_zoom: int, max_zoom: int,
        tooltip: str, link: str, stroke: bool, color: str, weight: float,
        opacity: float, lineCap: str, lineJoin: str, dashArray: str,
        dashOffset: str, fill: bool, fillColor: str, fillOpacity: float) -> None: ...

    # General properties
    
    basemap :str
    attribution: str
    leaflet_version: str
    leaflet_url: str
    leaflet_stylesheet_url: str
    min_zoom: int
    max_zoom: int
    
    # Element-specific properties

    tooltip: str
    link: str
    stroke: bool
    color: str
    weight: float
    opacity: float
    lineCap: str
    lineJoin: str
    dashArray: str
    dashOffset: str
    fill: bool
    fillColor: str
    fillOpacity: float

    def add(self, geom: Union['Box', 'Coordinate', 'Feature', Geometry], *, 
        tooltip: str, link: str, stroke: bool, color: str, weight: float,
        opacity: float, lineCap: str, lineJoin: str, dashArray: str,
        dashOffset: str, fill: bool, fillColor: str, fillOpacity: float) -> 'Map': ...
    def save(self, filename: str) -> None: ...
    def show(self) -> None: ...
    
class Tags:
    ...
    
class Tile:
    bounds: 'Box'
    column: int
    id: str
    revision: int
    row: int
    size: int
    zoom: int
