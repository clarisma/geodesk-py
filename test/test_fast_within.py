def test_performance_intersects(features):
    usa = features(
        "a[boundary=administrative]"
        "[admin_level=2][name='United States']").one
    buildings = features("a[building]")
    
    print(f"{buildings.within(usa).count} buildings in USA")
    print(f"{buildings.relations.within(usa).count} relation-buildings in USA")