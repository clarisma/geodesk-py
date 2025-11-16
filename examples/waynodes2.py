from geodesk import *

mc = Features("d:\\geodesk\\tests\\mcu.gol")
for w in mc("w[highway=primary]"):
    print(w)
    for node in w.nodes:
        print(f"- {node}")
