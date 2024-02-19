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
    print_timing("Best   ", best_time, count)
    print_timing("Median ", median_time, count)
    print_timing("Average", average_time, count)
    print_timing("Worst  ", worst_time, count)

def notest_crosses(features):
    map = Map()
    bavaria = features("a[boundary=administrative][admin_level=4][name:en=Bavaria]").one
    danube = features("r[waterway=river][name:en=Danube]").one
    map.add(danube)
    rail_bridges = features("w[railway][bridge]")
    results = rail_bridges.crosses(danube)(bavaria)
    for tile in results.tiles:
        map.add(tile.bounds, tooltip=str(tile), color="orange")
    map.add(results, color="red")
    map.show()
    
def test_crosses_speed(features):
    bavaria = features("a[boundary=administrative][admin_level=4][name:en=Bavaria]").one
    danube = features("r[waterway=river][name:en=Danube]").one
    rail_bridges = features("w[bridge]")
    benchmark("Bridges across Danube in Bavaria", lambda: rail_bridges.crosses(danube)(bavaria).count)
    