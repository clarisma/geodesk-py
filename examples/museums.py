# Lists the museums in Paris and all metro stations within 500 meters

import geodesk

world = geodesk.Features('c:\\geodesk\\tests\\france.gol')
paris = world("a[boundary=administrative][admin_level=8][name=Paris]").one
museums = world("na[tourism=museum]")
stations = world("n[railway=station]")
for museum in museums(paris):
    print(f"{museum.name}")
    for station in stations.around(museum, meters=500):
        print(f"- {station.name}")
