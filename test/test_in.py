

def test_contains(features):
    highways = features("w[highway]")
    sushi_restaurants = features("na[amenity=restaurant][cuisine=sushi]")
    restaurant = sushi_restaurants[0]
    assert restaurant in sushi_restaurants
    assert highways[0] not in sushi_restaurants
    