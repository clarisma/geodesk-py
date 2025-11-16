import geodesk

france = geodesk.Features('d:\\geodesk\\tests\\france.gol')

streets = france("w[highway][highway!=footway,path,steps][name]")
admin_areas = france("a[boundary=administrative][name]")
for street in streets:
    print(f"{street.name} is in:")
    for area in admin_areas.containing(street.nodes.first.centroid):
        print(f"- {area.name} ({area.admin_level})")
