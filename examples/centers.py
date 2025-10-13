import geodesk
import shapely

france = geodesk.Features('d:\\geodesk\\tests\\france.gol')
paris = france("a[boundary=administrative][admin_level=6][name=Paris]").one
museums = france("na[tourism=museum]")

for museum in museums(paris):
    c = museum.centroid
    print(f"{museum.name}: {c.lon}, {c.lat}")
    p = geodesk.from_mercator(shapely.point_on_surface(museum.shape))
    print(f"  Point-on-surface: {p.x}, {p.y}")
