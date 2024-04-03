# Copyright (c) 2024 Clarisma / GeoDesk contributors
# SPDX-License-Identifier: LGPL-3.0-only

import pytest
from geodesk import *

def show(features):
    for f in features[:20]:
        print(f"{f}: area = {f.area}, length = {f.length}")

def test_max_area(features):
    print("\nVery small houses:")
    show(features("a[building=house]").max_area(ft=300))
    print("\nExcessively small tennis courts:")
    show(features("a[leisure=pitch][sport=tennis]").max_area(ft=2000))
    
def test_min_area(features):
    print("\nVery large houses:")
    show(features("a[building=house]").min_area(2000))
    
    # 0 or negative must return all features
    assert features.count == features.min_area(0).count
    assert features.count == features.min_area(-10).count

def test_max_length(features):
    print("\nVery short streets:")
    show(features("w[highway]").max_length(1.5))
    print("\nExtremely short railways:")
    show(features("w[railway]").max_length(0.3))

def test_min_length(features):
    print("\nVery long streets:")
    show(features("w[highway]").min_length(km=2))

def test_invalid(features):
    with pytest.raises(TypeError):
        features.max_area(1,2)
    with pytest.raises(TypeError):
        features.max_area("nonsense")
    with pytest.raises(TypeError):
        features.max_area(bad_unit=44)
    with pytest.raises(TypeError):
        features.max_area(feet=12, meters=6)
    with pytest.raises(TypeError):
        features.max_area(km="bananas")
        
def test_adjacent(features):
    streets = features("w[highway=primary,secondary,tertiary,residential]") 
    for w in streets.max_length(1.5):
        for neighbor in streets.parents_of(w.nodes.first):
            if neighbor != w and dict(w.tags) == dict(neighbor.tags):
                print(f"{w} could be merged into {neighbor}")
                # TODO: Should also check relations
                
     