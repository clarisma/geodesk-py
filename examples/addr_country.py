"""
This script checks the addr:country tags of objects to ensure they're
actually in that country.

- addr:country should be the 2-digit ISO3166-1 code
- If the feature is in an admin_level=4 with its own ISO3166-1 code,
  check against that code instead (e.g. overseas territories of FR, NL)
  
Weird cases:
- node/9954800114: Is in AT but only accessible by road from DE, hence
  addr:country=DE is considered legal
  
"""

from geodesk import *

"""
allowed = { 
    "AW": "NL", # Aruba  
    "BQ": "NL", # Caribbean Netherlands
    "CW": "NL", # Curacao
    "EL": "GR", # Greece
    "GP": "FR", # Guadeloupe
    "MQ": "FR", # Martinique
    "NC": "FR", # New Caledonia
    "PF": "FR", # French Polynesia
    "RE": "FR", # Reunion
    "SX": "NL", # Sint Maarten
    "YT": "FR", # Mayotte 
    
    }
"""

allowed = { 
    "BQ": "NL", # Bonaire
    "EL": "GR", # Greece
}

error_count = 0

root_path = "c:\\geodesk\\tests\\"
world = Features(root_path + "w2")
countries = world("a[boundary=administrative][admin_level=2]['ISO3166-1']")
territories = world("a[boundary=administrative][admin_level=3]")
states = world("a[boundary=administrative][admin_level=4]")
country_features = world("[addr:country]")
count = 0

def name(feature):
    return feature['name:en'] or feature.name

with open(root_path + "addr_country_errors.txt", 'w', encoding='utf-8') as file:
    def report(feature, error, country, value):
        global error_count
        file.write(f"{feature},{error},{country},{value}\n")
        error_count += 1

    for country in countries:
        print(f"Checking features in {name(country)}...")
        country_code = country["ISO3166-1"]
        country_territories = {}
        country_states = {}
        for state in states.within(country):
            code = state["ISO3166-1"]
            if code:
                print(f"- {name(state)} is a territory of {name(country)}")
                country_territories[code] = world.within(state)
            else:
                code = state["ISO3166-2"]
                if code:
                    code = code[-2:]    # get last two letters
                    country_states[code] = world.within(state)
        for territory in territories.within(country):                
            code = territory["ISO3166-1"]
            if code:
                print(f"- {name(territory)} is a territory of {name(country)}")
                country_territories[code] = world.within(territory)

        for feature in country_features.within(country):
            count += 1
            feature_country_code = feature["addr:country"]
            if feature_country_code != country_code:
                try:
                    if allowed[feature_country_code] == country_code:
                        continue
                except KeyError:
                    pass
                try:
                    if feature in country_territories[feature_country_code]:
                        continue
                except KeyError:
                    pass
            
                try:
                    if feature in country_states[feature_country_code]:
                        report(feature, "addr_country_state_code", 
                            country_code, feature_country_code)
                        continue
                except KeyError:
                    pass
                report(feature, "addr_country_wrong_code", 
                    country_code, feature_country_code)

print(f"{error_count} of {count} features have wrong addr:country tag")        

    