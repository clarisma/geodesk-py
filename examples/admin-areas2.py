from geodesk import *

world = Features("d:\\geodesk\\tests\\world")
countries = world("a[boundary=administrative][admin_level=2][name]").relations

for country in countries:
    name = country['name:en']
    if name is None:
        name = country.name
    print(f"{country}: {name}:")
    for member in country.members:
        print(f"- {member}")


