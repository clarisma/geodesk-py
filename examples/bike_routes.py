import geodesk

world = geodesk.Features('c:\\geodesk\\tests\\world.gol')
burgundy = world("a[boundary=administrative][admin_level=4][ref:INSEE=27]").one

# Regional, national and international bike routes
bike_routes = world("r[route=bicycle][network=rcn,ncn,icn]")

for route in bike_routes(burgundy):
    total_length = route.length
    if total_length > 50000:
        nice_length = route.members("w[highway=path,cycleway,track]").length
        print(
            f"{route.name}: {nice_length / 1000:.1f} "
            f"of {total_length / 1000:.1f} ({nice_length / total_length * 100:.1f}%)")