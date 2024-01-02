# Copyright (c) 2024 Clarisma / GeoDesk contributors
# SPDX-License-Identifier: LGPL-3.0-only

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
    print("State                  Calculated   Wikipedia")
    print("--------------------   ----------   ----------")
    for name, official_area in zip(state_names, official_area):
        state = features(f"a[boundary=administrative][admin_level=4][name='{name}']").one
        area = round(state.area / 1000000)
        print(f"{name:20}   {area:>10}   {official_area:>10}")