def test_tiles(features):
    m = Map()
    for tile in features.tiles:
        bounds = tile.bounds
        m.add(bounds, tooltip=str(tile))
    m.show()    