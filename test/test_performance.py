# Copyright (c) 2024 Clarisma / GeoDesk contributors
# SPDX-License-Identifier: LGPL-3.0-only

import time
from geodesk import *

def print_timing(title, secs, count):
    if isinstance(count,int):
        print(f"  {title}: {secs:.6f} s ({count / secs / 1000000:.3f}M features/s)")
    else:
        print(f"  {title}: {secs:.6f} s")
    
def benchmark(desc, fun):
    timings = []
    for _ in range(10):
        start_time = time.time()
        count = fun()
        end_time = time.time()
        timings.append(end_time - start_time)

    timings.sort()
    average_time = sum(timings) / len(timings)
    median_time = timings[len(timings) // 2] if len(timings) % 2 == 1 else (timings[len(timings) // 2 - 1] + timings[len(timings) // 2]) / 2
    best_time = timings[0]
    worst_time = timings[-1]

    print("")
    print(f"{desc} ({count}):")   
    # print(f"  Best:    {best_time:.6f} s")
    # print(f"  Median:  {median_time:.6f} s")
    # print(f"  Average: {average_time:.6f} s")
    # print(f"  Worst:   {worst_time:.6f} s")
    print_timing("Best   ", best_time, count)
    print_timing("Median ", median_time, count)
    print_timing("Average", average_time, count)
    print_timing("Worst  ", worst_time, count)
    

def test_performance_intersects(features):
    germany = features(
        "a[boundary=administrative]"
        "[admin_level=2][name:en=Germany]").one
    bavaria = features(
        "a[boundary=administrative]"
        "[admin_level=4][name:en=Bavaria]").one
    usa = features(
        "a[boundary=administrative]"
        "[admin_level=2][name='United States']").one
    buildings = features("a[building=yes]")
    
    benchmark("Filter construction", 
        lambda: buildings.intersects(bavaria))
    
    benchmark("Buildings that intersect Bavaria", 
        lambda: buildings.intersects(bavaria).count)

    benchmark("Buildings that intersect Bavaria's bbox", 
        lambda: buildings(bavaria.bounds).count)
    
    benchmark("Buildings within Bavaria", 
        lambda: buildings.within(bavaria).count)
    
    benchmark("Buildings that intersect Germany", 
        lambda: buildings.intersects(germany).count)
    
    benchmark("Buildings within Germany", 
        lambda: buildings.within(germany).count)
    
    benchmark("Buildings that intersect USA", 
        lambda: buildings.intersects(usa).count)
    
    benchmark("Buildings within USA", 
        lambda: buildings.within(usa).count)
    
    benchmark("All post boxes", 
        lambda: features("na[amenity=post_box]").count)
    