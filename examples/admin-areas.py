from geodesk import *

world = Features("c:\\geodesk\\tests\\w")

top_key         = "name"
top_value       = "France"
top_admin_level = 2
max_admin_level = 12

top = world(
    f"a[boundary=administrative][{top_key}='{top_value}']"
    f"[admin_level={top_admin_level}]").one
sub_areas = world("a[boundary=administrative][admin_level][name]")
area_to_parent = {}
area_to_children = {}
areas_by_level = [[] for _ in range(max_admin_level + 1)]

print("Gathering all areas...")
for area in sub_areas(top):
    print(area.name)
    # areas_by_level[area.admin_level].append(area)

print("Assigning areas to parents...")
for level in range(1, max_admin_level + 1):
    for area in areas_by_level[level]:
        for sub_area in sub_areas(area):
            area_to_parent[sub_area] = area







