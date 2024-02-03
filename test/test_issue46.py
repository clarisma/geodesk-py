def test_issue46(features):
    nodes = features("n")
    for n in nodes:
         print(n)
         parentareaways = n.parents("a")