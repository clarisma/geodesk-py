
def test_filter(features):
    for street in features("w[highway]").filter(lambda f: len(f.tags) == 3):    
        print(street)
        assert len(street.tags) == 3