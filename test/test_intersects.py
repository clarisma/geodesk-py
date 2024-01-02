# Copyright (c) 2024 Clarisma / GeoDesk contributors
# SPDX-License-Identifier: LGPL-3.0-only

def test_intersects(features):
    """
    - An `intersects` query should return the same number of features,
     regardless whether we use the area feature or its shape
    - The short form `features(area)` creates an `intersects` filter
      under the hood, so those results should be the same as well
    """

    city = features("a[boundary=administrative][admin_level=6][name=Braunschweig]").one
    streets = features("w[highway]")
    count = streets.intersects(city).count
    assert count > 100
    assert streets.intersects(city.shape).count == count
    assert streets(city).count == count
    assert streets(city.shape).count == count
    in_city = features.intersects(city)
    assert streets(in_city).count == count
    assert (streets & in_city).count == count