# Copyright (c) 2024 Clarisma / GeoDesk contributors
# SPDX-License-Identifier: LGPL-3.0-only

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
