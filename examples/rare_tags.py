from geodesk import *

key = "highway"
min_occurrence = 20

world = Features('c:\\geodesk\\tests\\de3.gol')
counts = {}
for feature in world(f"[{key}]"):
    value = feature.str(key)
    if value in counts:
        counts[value] += 1
    else:
        counts[value] = 1
m = Map(link = "https://www.osm.org/edit?{osm_type}={id}", color="red")        
for value, count in counts.items():
    if count < min_occurrence:
        tooltip = f"{key}={value} ({count})"
        for feature in world(f'[{key}="{value}"]'):
            m.add(feature, tooltip=tooltip)
m.show()            

    