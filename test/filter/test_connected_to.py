# Copyright (c) 2024 Clarisma / GeoDesk contributors
# SPDX-License-Identifier: LGPL-3.0-only

from geodesk import *

def notest_connected_to(features):
    count = 0
    waterways = features("w[waterway=river,canal]")
    systems = []
    assigned = set()
    for w in waterways:
        count += 1
        if not w in assigned:
            system = []
            stack = []
            stack.append(w)
            while stack:
                w = stack.pop()
                assigned.add(w)
                system.append(w)
                for c in waterways.connected_to(w):
                    if c not in assigned:
                        stack.append(c)
            systems.append(system)            
    print (f"Found {count} waterways in {len(systems)} river systems.")  
    
    # m = Map(color="red")
    # m.add(systems[52])
    # m.show()
    