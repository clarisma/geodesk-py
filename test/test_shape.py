# Copyright (c) 2023 Clarisma / GeoDesk contributors
# SPDX-License-Identifier: LGPL-3.0-only

from geodesk import *
import pytest

def notest_features_shape(features):
    features("n[place=city][name=Berlin]").shape
    
def test_superroute_shape(features):
    route = features("r[route=bicycle][wikidata=Q3060537]").one
    route.shape
