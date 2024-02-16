from geodesk import Features, Box;
from shapely import *;

coords = [
    (13.3898033, 52.5174377),
    (13.4523542, 52.5110493),
    (-120, -40)]

germany = Features("c:\\geodesk\\tests\\berlin.gol")
streets = germany("w[highway][maxspeed]")

for c in coords:
    c_lon, c_lat = c
    box = Box(lon=c_lon, lat=c_lat).buffer(meters = 30)
    p = Point(box.centroid)
    nearby_streets = streets(box)
    closest_street = None
    closest_distance = 1000000
    for street in nearby_streets:
        d = p.distance(street.shape)
        if d < closest_distance:
            closest_street = street
            closest_distance = d
    if closest_street:
        print(
            f"Closest street to ({c_lon}, {c_lat}) is {closest_street.name} "
            f"at {from_mercator(closest_distance, unit='meters', lat=c_lat): .1f} meters, "
            f"with maxspeed of {closest_street.maxspeed}")
    else:
        print(f"Found no street near ({c_lon}, {c_lat})")