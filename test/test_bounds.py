from geodesk import *

def test_polar(features):
    """
    Averaging min/max Y (e.g. center point of bbox) previously 
    caused an overflow for values near the poles.
    """
    for f in features("war")(Box(s=80))[:100]:
        assert Coordinate(f.x, f.y) in f.bounds
    