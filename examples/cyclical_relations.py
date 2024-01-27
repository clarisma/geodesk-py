# Find all relations that contain a reference cycle

from geodesk import *

world = Features("c:\\geodesk\\tests\\w2.gol")

count = 0

def report_cycles(chain):
    global count
    count += 1
    print(f"\n{chain[0]} leads to a reference cycle:")
    indent = 0
    marker = ""
    loop_found = False
    for rel in chain:
        if rel == chain[-1]:
            # Mark the relation that forms a loop
            marker = "<-- " if loop_found else "--> "
            loop_found = True
        else:
            marker = ""
        role = f" as {rel.role}" if rel.role else ""
        print(f"{' ' * indent}{marker}{rel} (type={rel.type}){role}")
        indent += 2

def find_cycles(rel, chain):
    if rel in chain:
        # We've found a circular reference
        chain.append(rel)    
        report_cycles(chain)
    else:    
        chain.append(rel)    
        for child in rel.members.relations:
            find_cycles(child, chain)
    chain.pop()    

for rel in world.relations:
    if rel.members.relations:
        # Relation has other relations as members
        find_cycles(rel, [])
        
print(f"\nFound {count} relations that contain a reference cycle.")
