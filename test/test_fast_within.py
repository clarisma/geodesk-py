def test_performance_intersects(features):
    usa = features(
        "a[boundary=administrative]"
        "[admin_level=2][name='United States']").one
    buildings = features("a[building]")

    with open("c:\\geodesk\\debug\\slow-rels.txt", 'w') as file:
        for rel in buildings.relations.within(usa):
            file.write(f"{rel.id}\n")

def read_ids(file_path):
    ids = set()
    with open(file_path, 'r') as file:
        for line in file:
            id = line.strip()
            if id.isdigit():
                ids.add(int(id))
    return ids

# Function to compare set A and set B (which both contain only integers) and prints the numbers from A that are not in B
def compare_sets_and_print_difference(set_a, set_b):
    difference = set_a - set_b
    for id in difference:
        print(id)

