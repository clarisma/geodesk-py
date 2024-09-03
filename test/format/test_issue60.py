# Copyright (c) 2024 Clarisma / GeoDesk contributors
# SPDX-License-Identifier: LGPL-3.0-only

from geodesk import *

def test_format_wide_num(features):
    f = features("n[place][name=Sjømannsbyen]").one
    assert f["ssr:stedsnr"] == 118031
    assert f.num("ssr:stedsnr") == 118031
    assert f.str("ssr:stedsnr") == '118031'