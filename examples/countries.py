# Exports the borders for counties in a given state

import geodesk

class Country:
    def __init__(self, code, name):
        self.code = code
        self.name = name
        self.alt_codes = set()
        
    def __repr__(self):
        return f"{self.code}: {self.name}: {self.tiles}"

def test_countries():
    world = geodesk.Features('c:\\geodesk\\tests\\w2.gol')
    countries = world("a[boundary=administrative][admin_level=2][name]['ISO3166-1']")
    list = []
    for country in countries:
        int_name = country["name:en"]
        if not int_name:
            int_name = country.int_name
        code = country["ISO3166-1"]    
        c = Country(code, int_name)
        list.append(c)
        c.tiles = len(world(country).tiles)
    list.sort(key=lambda obj: obj.code)
    print(list)
    print(f"{len(list)} countries found.")        
test_countries()