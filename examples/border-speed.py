from geodesk import *
import time

countries = [
    ("Brazil", -15, -50),
    ("Canada", 53, -112),
    ("China", 35, 100),
    ("France", 47, 4),
    ("Germany", 51, 10),
    ("Russia", 56, 38),
    ("United States", 39, -96),
]

world = Features('c:\\geodesk\\tests\\world.gol')
for _ in range(1,10):
    start_time = time.perf_counter()
    for country_name, lat, lon in countries:
        # print(country_name)
        country = world("a[boundary=administrative][admin_level=2][name:en='" + 
            country_name + "']")(Box(s=lat, n=lat, w=lon, e=lon)).one
        country.shape
    end_time = time.perf_counter()
    elapsed_time = end_time - start_time
    print(f"Retrieved {len(countries)} country borders in {elapsed_time:.3f} seconds")