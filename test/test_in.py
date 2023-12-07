

def test_contains(features):
    highways = features("w[highway]")
    sushi_restaurants = features("na[amenity=restaurant][cuisine=sushi]")
    restaurant = sushi_restaurants[0]
    street = highways[0]
    assert restaurant in sushi_restaurants
    assert street not in sushi_restaurants
    for node in street.nodes:
        assert node in street.nodes
        assert node not in sushi_restaurants
    
def test_contains_member(features):
    routes = features("r[route]")
    route = routes[0]
    for member in route.members:
        assert member in route.members
        assert route in member.parents
        assert route in member.parents.relations