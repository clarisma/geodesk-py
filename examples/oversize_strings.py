from geodesk import *

def check_string(f, type, s):
    # l = len(s.encode('utf-8')) 
    l = len(s) 
    if l > 255:
        print(f"{f}: Excess {type} length ({l}): {s}")
    
world = Features("c:\\geodesk\\tests\\de")
count = 0

for f in world:
    for k,v in f.tags:
        check_string(f, "key", str(k))
        check_string(f, "value", str(v))
        count += 2

print(f"Checked {count} strings.")