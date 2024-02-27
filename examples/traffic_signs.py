# See https://community.openstreetmap.org/t/large-scale-change-of-traffic-sign-to-traffic-sign-id/107508/117

from geodesk import *

total_count = 0
highway_member_count = 0
world = Features("c:\\geodesk\\tests\\w2")
traffic_signs = world("n[traffic_sign]")
for sign in traffic_signs:
    total_count += 1
    if sign.parents.ways("*[highway]"):
        highway_member_count += 1

print(f"Of {total_count} traffic signs, {highway_member_count} belong to highways")

