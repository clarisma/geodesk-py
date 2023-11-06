# Inspired by https://github.com/amandasaurus/waterwaymap.org,
# this script discovers which waterways are connected

import geodesk
import time

world = geodesk.Features('c:\\geodesk\\tests\\de.gol')

start_time = time.perf_counter()
count = 0
waterways = world("wa[waterway=river,canal,stream], wa[natural=water]")
    # For now, all selectors must have same type (e.g. "wa" in this case)
    # See GitHub issue geodesk-py#15

systems = []
assigned = set()
for w in waterways:
    count += 1
    if not w in assigned:
        system = []
        stack = []
        stack.append(w)
        while stack:
            w = stack.pop()
            assigned.add(w)
            system.append(w)
            for c in waterways.connected_to(w):
                if c not in assigned:
                    stack.append(c)
        systems.append(system)            
print (f"Found {count} waterways in {len(systems)} river systems.")  

end_time = time.perf_counter()
elapsed_time = end_time - start_time
print(f"Elapsed time: {elapsed_time:.1f} seconds")

for i in range(60,67):
    m = geodesk.Map()
    m.add(systems[i], color="red", link="https://www.openstreetmap.org/edit?{osm_type}={id}").show()