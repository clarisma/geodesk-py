from shapely import Geometry
from typing import Callable, Dict, Iterator, List, Optional, Sequence, Tuple, Union, overload

class Box:
    def __init__(self, /, minx: int, miny: int, maxx: int, maxy: int, *,
        minlon: float, minlat: float, maxlon: float, maxlat: float,
        left: int, right: int, top: int, bottom: int, 
        west: float, south: float, east: float, north: float,  
        w: float, s: float, e: float, n: float,
        x: int, y: int, lon: float, lat: float) -> None: ...
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
    def __init__(self, /, x: int=None, y: int=None, *, lon: float, lat: float) -> None: ...
    x: int
    y: int
    lon: float
    lat: float

class Feature:
    area: float
    bounds: 'Box'
    centroid: 'Coordinate'
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
    def containing(self, geom: Union['Box', 'Coordinate', 'Feature', Geometry]) -> 'Features': ...
    def contained_by(self, geom: Union['Box', 'Coordinate', 'Feature', Geometry]) -> 'Features': ...
    def crossing(self, geom: Union['Box', 'Coordinate', 'Feature', Geometry]) -> 'Features': ...
    def disjoint_from(self, geom: Union['Box', 'Coordinate', 'Feature', Geometry]) -> 'Features': ...
    def intersecting(self, geom: Union['Box', 'Coordinate', 'Feature', Geometry]) -> 'Features': ...
    def max_area(self, n:float=None, *, 
        meters:float, m:float, feet:float, ft:float, km:float, miles:float) -> 'Features': ...
    def min_area(self, n:float=None, *, 
        meters:float, m:float, feet:float, ft:float, km:float, miles:float) -> 'Features': ...
    def max_length(self, n:float=None, *, 
        meters:float, m:float, feet:float, ft:float, km:float, miles:float) -> 'Features': ...
    def min_length(self, n:float=None, *, 
        meters:float, m:float, feet:float, ft:float, km:float, miles:float) -> 'Features': ...
    def members_of(self, feature: 'Feature') -> 'Features': ...
    def nodes_of(self, feature: 'Feature') -> 'Features': ...
    def overlapping(self, geom: Union['Box', 'Coordinate', 'Feature', Geometry]) -> 'Features': ...
    def parents_of(self, feature: 'Feature') -> 'Features': ...
    def touching(self, geom: Union['Box', 'Coordinate', 'Feature', Geometry]) -> 'Features': ...
    def within(self, geom: Union['Box', 'Coordinate', 'Feature', Geometry]) -> 'Features': ...
    def __and__(self, other: "Features") -> "Features": ...
    def __iter__(self) -> Iterator['Feature']: ...
    def __contains__(self, item: 'Feature') -> bool: ...
    def __call__(self, arg: Union[str ,Box, 'Coordinate', 'Features']) -> Features: ...
    
class Formatter:
    id: Union[str, Callable[['Feature'], Union[str,int]]]
    limit: int
    linewise: bool
    mercator: bool
    precision: int
    pretty: bool
    def save(self, filename: str) -> None: ...

class Map:
    def __init__(self, data: Optional[str] = ..., *,
        basemap: Optional[str] = ..., attribution: Optional[str] = ..., leaflet_version: Optional[str] = ...,
        leaflet_url: Optional[str] = ..., leaflet_stylesheet_url: Optional[str] = ..., min_zoom: Optional[int] = ...,
        max_zoom: Optional[int] = ..., tooltip: Optional[str] = ..., link: Optional[str] = ...,
        stroke: Optional[bool] = ..., color: Optional[str] = ..., weight: Optional[float] = ...,
        opacity: Optional[float] = ..., lineCap: Optional[str] = ..., lineJoin: Optional[str] = ...,
        dashArray: Optional[str] = ..., dashOffset: Optional[str] = ..., fill: Optional[bool] = ...,
        fillColor: Optional[str] = ..., fillOpacity: Optional[float] = ...) -> None: ...

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
    def __iter__(self) -> Iterator[Tuple[str, Union[str,int,float]]]: ...
    
class Tile:
    bounds: 'Box'
    column: int
    id: str
    revision: int
    row: int
    size: int
    zoom: int

def to_mercator(geom: Union['Box', 'Coordinate', 'Feature', Geometry]=None, *,
    meters: float, m: float, feet: float, ft: float, km: float, miles: float,                
    lat: float, y: int) -> Union['Box', 'Coordinate', 'Feature', Geometry, int]: ...

def from_mercator(geom: Union['Box', 'Coordinate', 'Feature', Geometry, int],
    uinits: str, lat: float, y: int) -> Union['Box', 'Coordinate', 'Feature', Geometry, float]: ...                  

@overload
def lonlat(coords: Union[List[float], List[Sequence[float]]]) -> List['Coordinate']: ...

@overload
def lonlat(*coords: float) -> List['Coordinate']: ...

@overload
def latlon(coords: Union[List[float], List[Sequence[float]]]) -> List['Coordinate']: ...

@overload
def latlon(*coords: float) -> List['Coordinate']: ...
