import geodesk

world = geodesk.Features('c:\\geodesk\\tests\\de.gol')

berlin = world("a[boundary=administrative][admin_level=4][name=Berlin]").one
streets = world("w[highway][name]")

street_lengths = {}
for street in streets(berlin):
    name = street.name
    street_lengths[name] = street_lengths.get(name, 0) + street.length

for name, length in sorted(street_lengths.items()):
    print (f"{name}, {round(length)}")
    
