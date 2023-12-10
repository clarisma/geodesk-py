
def test_filter(features):
    count = 0
    for street in features("w[highway=primary]").filter(lambda f: len(f.tags) == 3):    
        # assert len(street.tags) == 3
        count += 1
    print(f"{count} primary streets have 3 tags")    
    
def test_filter_manual(features):
    count = 0
    for street in features("w[highway=primary]"):
        if len(street.tags) == 3:
            count += 1
    print(f"{count} primary streets have 3 tags")    
