# SPDX-License-Identifier: LGPL-3.0-only

from geodesk import *
import pytest

@pytest.fixture(scope="module")
def features():
    f = Features('c:\\geodesk\\tests\\de.gol')
    yield f

def test_rtree(features):
    bavaria = features("a[boundary=administrative][admin_level=4][name:en=Bavaria]").one
    # munich = features("n[place=city][name:en=Munich]")
    # restaurants = features("na[amenity=restaurant][cuisine=sushi,japanese]")
    # buildings = features("a[building]")
    soccer = features("a[leisure=pitch][sport=soccer]")
    print(f'Intersects Bavaria     : {soccer.intersects(bavaria).count}')
    # print(f'Intersects Bavaria bbox: {restaurants(bavaria.bounds).count}')
    # print(f'Intersects Bavaria     : {restaurants.intersects(bavaria).count}')
    # print(f'Intersects Bavaria bbox: {restaurants(bavaria.bounds).count}')
    # print(f'Intersects Bavaria     : {munich.intersects(bavaria).count}')
    # print(f'Intersects Bavaria bbox: {munich(bavaria.bounds).count}')
    # tree = RTree(restaurants)    

