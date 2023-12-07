from geodesk import *
import time

world = Features('c:\\geodesk\\tests\\world.gol')
for _ in range(1,10):
    start_time = time.perf_counter()
    count = world("w[highway]").count   # =residential][maxspeed]
    end_time = time.perf_counter()
    elapsed_time = end_time - start_time
    print(f"Retrieved {count} features in {elapsed_time:.3f} seconds")