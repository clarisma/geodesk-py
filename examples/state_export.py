# Exports the borders for counties in a given state

import geodesk
import sys

world = geodesk.Features('c:\\geodesk\\tests\\world.gol')
# usa = world("a[boundary=administrative][admin_level=2][name='United States']").one
state_name = sys.argv[1] # first command-line argument (e.g. "California")
file_name = state_name.lower().replace(' ', '-') + "-counties"
state = world(f"a[boundary=administrative][admin_level=4][name='{state_name}']").one
counties = world("a[boundary=administrative][admin_level=6]").within(state)
counties.map(file_name, tooltip="{name}").show()
counties.geojson.save(file_name)