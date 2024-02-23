from geodesk import *

world = Features("c:\\geodesk\\tests\\w2")
longest_key = ""
max_key_len = 0
longest_value = ""
max_value_len = 0
tag_count = 0

for f in world:
    for k,v in f.tags:
        k = str(k)
        v = str(v)
        key_len = len(k)
        value_len = len(v)
        if key_len > max_key_len:
            max_key_len = key_len
            longest_key = k
        if value_len > max_value_len:
            max_value_len = value_len
            longest_value = v
        tag_count += 1
        if tag_count % 1000000 == 0:
            print(f"Checked {tag_count} tags...\r", end='')

print(f"\nChecked {tag_count} tags.      ")
print(f"Longest key is   {longest_key} ({max_key_len})")            
print(f"Longest value is {longest_value} ({max_value_len})")            