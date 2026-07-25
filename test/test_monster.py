def test_relation_6535292(world):
    tongass = world("a[boundary=protected_area][name='Tongass National Forest']").one
    print(tongass.name)
    print(tongass.tags)
    shape = tongass.shape
    str(tongass.geojson)
    str(tongass.wkt)