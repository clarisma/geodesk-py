# See https://community.openstreetmap.org/t/large-scale-change-of-traffic-sign-to-traffic-sign-id/107508/117

import time
from geodesk import *

total_count = 0
highway_member_count = 0
world = Features("c:\\geodesk\\tests\\w3")

def query1():
    global total_count
    global highway_member_count 
    traffic_signs = world("n[traffic_sign]")
    for sign in traffic_signs:
        total_count += 1
        if sign.parents.ways("[highway]"):
            highway_member_count += 1

def query2():
    global total_count
    global highway_member_count 
    traffic_signs = world("n[traffic_sign]")
    highways = world.ways("[highway]")
    for sign in traffic_signs:
        total_count += 1
        if highways.parents_of(sign):
            highway_member_count += 1


start_time = time.time()
query2()        
print(f"Of {total_count} traffic signs, {highway_member_count} belong to highways")
print(f"Query took {time.time()-start_time} seconds")

