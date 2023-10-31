# Displays the 10 most common collection times for post boxes

import geodesk
from collections import Counter

world = geodesk.Features('c:\\geodesk\\tests\\world.gol')
post_boxes = world("na[amenity=post_box]")
count = Counter([box.collection_times for box in post_boxes])
most_common = count.most_common(10)
for collection_times, occurrences in most_common:
    print(f"{occurrences:>10} : {collection_times}")