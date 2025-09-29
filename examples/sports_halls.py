# Finds all sports halls and sports centers, and outputs their names,
# locations and cities as CSV. If a sports hall doesn't have an explicit
# addr:city, we look for the city where it is located

from geodesk import *


germany  = Features('c:\\geodesk\\tests\\de.gol')
sports_halls = germany("na[building=sports_hall,sports_centre], na[building][leisure=sports_centre,sports_hall]")
cities   = germany("a[boundary=administrative][admin_level=8]")
counties = germany("a[boundary=administrative][admin_level=6]")
states   = germany("a[boundary=administrative][admin_level=4]")

def find_city(location):
    city = cities.containing(location).first
    if not city:
        city = counties.containing(location).first
        if not city:
            city = states.containing(location).first
    return city

print("name,lon,lat,city")
for hall in sports_halls:
    location = hall.centroid
    city_name = hall["addr:city"]
    if city_name is None:
        city = find_city(location)
        if city:
            city_name = city.name
        if city_name is None:
            city_name = "Unknown"
    print(f'"{hall.name}",{location.lon},{location.lat},"{city_name}"')