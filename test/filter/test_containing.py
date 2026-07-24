# Copyright (c) 2024 Clarisma / GeoDesk contributors
# SPDX-License-Identifier: LGPL-3.0-only

from geodesk import *

def name_set(features):
    return { f.str('name') for f in features }

def test_containing_point(features):
    city = features("n[place=city][name=Heidelberg]").one
    areas = features("a[boundary]")
    required = {
        "Deutschland", 
        "Regierungsbezirk Karlsruhe",
        "Baden-W\u00FCrttemberg",
        "Verkehrsverbund Rhein-Neckar",
        # "Region Rhein-Neckar (BW)",
        "Altstadt" }
    names1 = name_set(areas.containing(city))
    assert required <= names1
    
    names2 = name_set(areas.containing(city.centroid))
    assert names1 == names2

def test_containing(features):
    streets = features("w[highway=primary][name]")
    admin_areas = features("a[boundary=administrative][name]")
    count = 0
    for street in streets:
        print(f"{street.name} is in:")
        for area in admin_areas.containing(street.nodes.first):
            print(f"- {area.name} ({area.admin_level})")
        count += 1
        if count == 10:
            break

def notest_containing_partial():
    features = Features('d:\\geodesk\\tests\\bavaria-from-world.gol')
    c = latlon(48.13739,11.57543)
    print([f.name for f in features("a[boundary=administrative]").containing(c)])
