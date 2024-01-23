def test_relation_6535292(features):
    tongass = features("a[boundary=protected_area][name='Tongass National Forest']").one
    shape = tongass.shape
    str(tongass.geojson)
    str(tongass.wkt)