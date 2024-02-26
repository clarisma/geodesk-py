

def notest_issue50(features):
    count = 0
    traffic_signs = features("n[traffic_sign]")
    for sign in traffic_signs:
        if not sign.parents.ways("*[highway]"):
            count += 1
    print(f"{count} traffic signs that are not part of a highway")
            
def notest_issue50_relation_nodes(features):
    traffic_sign_nodes_on_non_highways = set()
    for h in features.relations("aw[!highway]"):
        # print(h)
        traffic_sign_nodes_on_non_highways.update(h.nodes("n[traffic_sign]"))