from geodesk2 import *

def test_change(features):
    f = features("w[highway]").first
    changes = Changes()
    print(changes[f].tags)
