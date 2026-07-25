# Copyright (c) 2024 Clarisma / GeoDesk contributors
# SPDX-License-Identifier: LGPL-3.0-only

from geodesk import *

def test_areas(features):    
    areas = [
        ("Araucania (Chile)", 31842.3, "a[boundary=administrative][admin_level=4][wikidata=Q2176]"),
        ("Baffin Island", 507451, "a[place=island][name:en='Baffin Island']"),
        ("Bavaria", 70550.19, "a[boundary=administrative][admin_level=4][name:en=Bavaria]"),
        ("Germany (land area)", 357600, "a[boundary=land_area][name:en='Federal Republic of Germany (land mass)']"),
        ("Greenland (admin area)", 2166086, "a[boundary=administrative][admin_level=2][name:en=Greenland]"),
        ("Komsomolets Island", 9006, "a[place=island][wikidata=Q248654]"),
        ("Meighen Island", 955, "a[place=island][wikidata=Q477759]"),
        ("Nordaustlandet", 14443, "a[place=island][name='Nordaustlandet']"),
        ("Tarapaca (Chile)", 41799.5, "a[boundary=administrative][admin_level=4][wikidata=Q2114]"),
        ("Saarland (Germany)", 2570, "a[boundary=administrative][admin_level=4][name=Saarland]"),
        ("Sakhalin (Russia)", 72492, "a[place=island][wikidata=Q7792]"),
        ("Victoria Island (Canada)", 217291, "a[place=island][name='Victoria Island'][wikidata=Q158129]"),
        ("Victoria Island (Russia)", 10.8, "a[place=island][name:en='Victoria Island'][wikidata=Q1276972]"),
    ]
    
    # For now, we've disabled the Lambert-projected area calculation, 
    # so both "fast" and "accurate" use the same Mercator-scaled Euclidian 
    # area calculation, which has a reasonable level of accuracy and 
    # is 5x faster than Lambert

    print() 
    print("                            Area (in square km)")
    print("Name                        Fast         Accurate     Wikipedia")
    print("-------------------------   ----------   ----------   ----------")
    for name, official_area, query in areas:
        try:
            f = features(query).one
        except QueryError:
            continue
        area = f.area / 1000000
        fast_area = area # f.fast_area / 1000000
        print(f"{name:25}   {fast_area:10.2f}   {area:10.2f}   {official_area:10.2f}")

    