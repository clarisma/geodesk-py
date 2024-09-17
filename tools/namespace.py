import os
import argparse
import re

"""
Usage: python tools/namespace.py <parth> <namespace_name>

Inserts namespace declarations into all .cpp and .h files in the given path
(including subdirectories), replacing any existing namespace declarations.

Namespace declarations are inserted after the last #include directive:

// Header and include statements

namespace my::cool::namespace {

// ... existing code ...

} // namespace my::cool::namespace

(Any existing namespace declarations must follow the same form 
or else they may not be recodgnized)
"""

def modify_file(file_path, namespace_name):
    with open(file_path, 'r') as f:
        lines = f.readlines()

    # Regular expression patterns for identifying any existing namespace
    namespace_start_pattern = re.compile(r'^\s*namespace\s+[\w:]+\s*\{')
    namespace_end_pattern = re.compile(r'^\s*\}\s*//\s*namespace\s+[\w:]+\s*')
    include_pattern = re.compile(r'^\s*#include\s+["<].+[">]')
    # Identify where the includes end and whether the file already contains namespace lines

    include_end_index = 0
    start_namespace_index = -1
    end_namespace_index = -1

    for i, line in enumerate(lines):
        if include_pattern.match(line):
            include_end_index = i + 1
        elif start_namespace_index == -1 and namespace_start_pattern.match(line):
            start_namespace_index = i
        elif namespace_end_pattern.match(line):
            end_namespace_index = i

    namespace_start = f"namespace {namespace_name} {{\n"
    namespace_end = f"}} // namespace {namespace_name}\n"

    if start_namespace_index != -1:
        assert end_namespace_index != -1
        # Replace existing namespace declarations
        modified_lines = lines
        modified_lines[start_namespace_index] = namespace_start
        modified_lines[end_namespace_index] = namespace_end
    else:
        # Insert new namespace declarations
        modified_lines = []
        for i, line in enumerate(lines):
            if i == include_end_index:
                modified_lines.append("\n")
                modified_lines.append(namespace_start)
            modified_lines.append(line)
        modified_lines.append("\n")
        modified_lines.append(namespace_end)

    # Write the modified content back to the file
    with open(file_path, 'w') as f:
        f.writelines(modified_lines)

def process_directory(path, namespace_name):
    for root, _, files in os.walk(path):
        for file in files:
            if file.endswith('.h') or file.endswith('.cpp'):
                file_path = os.path.join(root, file)
                modify_file(file_path, namespace_name)
                print(f"Modified: {file_path}")

def main():
    parser = argparse.ArgumentParser(description="Add or update namespace in .h and .cpp files")
    parser.add_argument('path', type=str, help='Relative path to the directory')
    parser.add_argument('namespace', type=str, help='Namespace name to insert')

    args = parser.parse_args()

    # Process the specified directory
    process_directory(args.path, args.namespace)

if __name__ == "__main__":
    main()
