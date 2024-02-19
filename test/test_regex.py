

def test_regex(features):
    s2 = features("w[highway][name~'^[AB].*']")
    s1 = features("w[highway][name=A*,B*]")
    count1 = s1.count
    count2 = s2.count
    assert count1 > 0
    assert count1 == count2
    
def test_regex2(features):
    s2 = features("w[highway][name~'Berliner Ring']")
    s1 = features("w[highway][name='Berliner Ring']")
    count1 = s1.count
    count2 = s2.count
    assert count1 > 0
    assert count1 == count2
