# Extracts all attribute names from a <Class>_attr.txt file and 
# creates a C++ source file with these attributes as a list of strings
# (This is the basis for __dir__)

import sys

def extract_attributes(filename):
    with open(filename, 'r') as file:
        lines = file.readlines()

    # Find the starting point (%%)
    start_index = next((i for i, line in enumerate(lines) if '%%' in line), None)
    if start_index is None:
        return []

    # Extract keywords from each line after '%%' until a comma is found
    attributes = []
    for line in lines[start_index+1:]:
        attr = line.split(',')[0].strip()
        if attr:
            attributes.append(attr)
    return attributes

def write_to_output(attributes, output_filename):
    content = [
        'static const int ATTR_COUNT = {};'.format(len(attributes)),
        'static const char* ATTR_NAMES[] =',
        '{'
    ]

    for attr in attributes:
        content.append('    "{}",'.format(attr))
    content.append('};')

    with open(output_filename, 'w') as file:
        file.write('\n'.join(content))

if __name__ == "__main__":
    attributes = extract_attributes(sys.argv[1])
    write_to_output(attributes, sys.argv[2])
