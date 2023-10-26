# Copyright (c) 2023 Clarisma / GeoDesk contributors
# SPDX-License-Identifier: LGPL-3.0-only

def name_set(features):
    return { f.str('name') for f in features }

def test_contains_point(features):
    city = features("n[place=city][name=Heidelberg]").one
    areas = features("a[boundary]")
    required = {
        "Deutschland", 
        "Regierungsbezirk Karlsruhe",
        "Baden-W\u00FCrttemberg",
        "Verkehrsverbund Rhein-Neckar",
        "Umweltzone Heidelberg",
        "Altstadt" }
    names1 = name_set(areas.contains(city))
    assert required <= names1
    
    names2 = name_set(areas.contains(city.centroid))
    assert names1 == names2
