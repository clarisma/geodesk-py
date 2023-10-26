# Copyright (c) 2023 Clarisma / GeoDesk contributors
# SPDX-License-Identifier: LGPL-3.0-only

from geodesk import *
import pytest

def test_init():
    c = Coordinate(100, 300)
    assert c.x == 100
    assert c.y == 300
    assert c[0] == 100
    assert c[1] == 300
    
    assert c == (100, 300)
    assert c == [100, 300]
    assert c != (100)
    assert c != [100]
    assert c != (100, 300, 2000)
    assert c != [100, 300, 2000]
    
    c2 = Coordinate(x=100, y=300)
    assert c == c2
    assert c2 == c
   
    c3 = Coordinate()
    assert c3.x == 0 and c3.y == 0
    assert c3 == (0,0)
    assert c3 != c

def test_init_with_wrong_args():
    with pytest.raises(ValueError) as ex_info:            
        bad = Coordinate(lon=190, lat=-100)
    assert "range" in str(ex_info.value)
    with pytest.raises(TypeError):            
        bad = Coordinate(set())
    with pytest.raises(TypeError):            
        bad = Coordinate(x="bad")
    
def test_as_tuple():
    c = Coordinate(7000, 4000)        
    x,y = c
    assert x == c.x
    assert y == c.y


