# Copyright (c) 2024 Clarisma / GeoDesk contributors
# SPDX-License-Identifier: LGPL-3.0-only

import time

def test_area_against_known(features):
    state_names = [ 
        "Baden-W\u00FCrttemberg",
        "Bayern", 
        "Berlin", 
        "Brandenburg", 
        "Hamburg", 
        "Hessen", 
        "Niedersachsen",
        "Rheinland-Pfalz",
        "Saarland",
        "Th\u00FCringen"]
    
    official_area = [   # from https://en.wikipedia.org/wiki/States_of_Germany
        35752,
        70552, 
        892, 
        29480,
        755, 
        21115, 
        47609,
        19853,
        2569,
        16172]
    
    # Hamburg area is off because official area is *land area*; OSM includes parts in the North Sea
    # same issue for Lower Saxony
    # Test is only useful for landlocked states
    
    print() 
    print("                       Area (in square km)")
    print("State                  Fast         Accurate     Wikipedia")
    print("--------------------   ----------   ----------   ----------")
    for name, official_area in zip(state_names, official_area):
        state = features(f"a[boundary=administrative][admin_level=4][name='{name}']").one
        fast_area = round(state.fast_area / 1000000)
        area = round(state.area / 1000000)
        print(f"{name:20}   {fast_area:>10}   {area:>10}   {official_area:>10}")
        
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

def sum_areas(list):
    area = 0
    for f in list:
        area += f.area
    return area    

def sum_areas_fast(list):
    area = 0
    for f in list:
        area += f.fast_area
    return area    

def notest_performance(features):
    areas = features("a")[:100000]
    benchmark("Fast area", lambda: sum_areas_fast(areas))
    benchmark("Accurate area", lambda: sum_areas(areas))

def test_polar_areas(features):
    greenland = features(
        "a[boundary=administrative]"
        "[admin_level=2][name:en=Greenland]").one
    print(f"Area of Greenland (fast):     {greenland.fast_area}")
    print(f"Area of Greenland (accurate): {greenland.area}")
    
    baffin = features("a[place=island][name:en='Baffin Island']").one
    print(f"Area of Baffin Island (fast):     {baffin.fast_area}")
    print(f"Area of Baffin Island (accurate): {baffin.area}")
    
    f = features("a[place=island][name='Nordaustlandet']").one
    print(f"Area of Nordaustlandet (fast):     {f.fast_area}")
    print(f"Area of Nordaustlandet (accurate): {f.area}")
    
    f = features("a[place=island][name:en='Victoria Island'][wikidata=Q1276972]").one
    print(f"Area of Victoria Island (fast):     {f.fast_area}")
    print(f"Area of Victoria Island (accurate): {f.area}")
    
    f = features("a[place=island][name='Meighen Island']").one
    print(f"Area of Meighen Island (fast):     {f.fast_area}")
    print(f"Area of Meighen Island (accurate): {f.area}")

    


    