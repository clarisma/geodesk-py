from geodesk import *

def test_polar(features):
    for f in features("war")(Box(s=80)):
        print(f"{f} at {f.lon}, {f.lat}")
        assert Coordinate(f.x, f.y) in f.bounds
    