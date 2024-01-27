# Find all empty relations

from geodesk import *

world = Features("c:\\geodesk\\tests\\world.gol")

count = 0
for rel in world.relations:
    for child in rel.members.relations:
        if not child.members:
            print(f"Empty: {child}")
            count += 1
        
print(f"\nFound {count} empty relations.")
