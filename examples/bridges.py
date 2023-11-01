# Highlights all railway bridges that cross the Danube in Bavaria

import geodesk

germany = geodesk.Features('c:\\geodesk\\tests\\de.gol')
bavaria = germany("a[boundary=administrative][admin_level=4][name:en=Bavaria]").one
danube = germany("r[waterway=river][name:en=Danube]").one
rail_bridges = germany("w[railway][bridge]")
rail_bridges(bavaria).crosses(danube).map("rail-crossings", color="red", weight=8, opacity=0.5).show()
