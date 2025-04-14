# Copyright (c) 2024 Clarisma / GeoDesk contributors
# SPDX-License-Identifier: LGPL-3.0-only

import geodesk


def test_countries():
    world = geodesk.Features('c:\\geodesk\\tests\\w.gol')
    countries = world("a[boundary=administrative][admin_level=2][name]")
    for country in countries:
        name = country.name
        int_name = country.int_name
        if not int_name:
            int_name = country["name:en"]
        code = country["ISO3166-1"]    
        s = "- "
        if code:
            s += code + " "
        s += name
        if int_name:
            s+= " (" + int_name + ")"
        print(s)