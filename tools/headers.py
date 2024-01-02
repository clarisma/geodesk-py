import os

# The directory where your C++ source and header files are located
ROOT_PATH = 'c:\\dev\\geodesk-py'
SOURCE_PATHS = [ ROOT_PATH + '\\src', ROOT_PATH + '\\test' ]

# The text block that you want to add at the beginning of each file
CPP_PREFIX_TEXT = """// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only
\n"""

PY_PREFIX_TEXT = """# Copyright (c) 2024 Clarisma / GeoDesk contributors
# SPDX-License-Identifier: LGPL-3.0-only
\n"""

HEADERS = { 
    ".cpp": CPP_PREFIX_TEXT,
    ".cxx": CPP_PREFIX_TEXT, 
    ".h":   CPP_PREFIX_TEXT, 
    ".hxx": CPP_PREFIX_TEXT, 
    ".py":  PY_PREFIX_TEXT, 
}

def should_process_file(filename):
    """
    Check if the file should be processed based on its extension.
    """
    return filename.endswith('.cpp') or filename.endswith('.cxx') or filename.endswith('.h')

def process_content(content, header_text):
    """
    Process the content of each file.
    """
    lines = content.split('\n')
    processed_lines = []

    n = header_text.find(' ')
    if n < 0: 
        raise RuntimeError("Cannot find comment start token")
    comment_start_token = header_text[0:n+1]

    # Flag to indicate that we've started code (non-comment, non-blank)
    code_started = False

    for line in lines:
        stripped = line.strip()

        if not code_started:
            if not stripped or stripped.startswith(comment_start_token):
                continue  # Skip this line because we haven't reached the code yet
            else:
                code_started = True  # Non-blank, non-comment line found

        # If we reached this point, it's either code or comments within the code
        processed_lines.append(line)

    # Join the processed lines back together and add the prefix text
    return header_text + '\n'.join(processed_lines)

def process_file(filepath, header_text):
    """
    Process each file: remove specific lines and prepend a text block.
    """
    with open(filepath, 'r') as file:
        content = file.read()
    new_content = process_content(content, header_text)
    with open(filepath, 'w') as file:
        file.write(new_content)

def main():
    count = 0
    for src_path in SOURCE_PATHS:
        for root, _, files in os.walk(src_path):
            for filename in files:
                n = filename.rfind('.')
                if n > 0:
                    extension = filename[n:]
                    try:
                        header_text = HEADERS[extension]
                    except KeyError:    
                        continue
                    file_path = os.path.join(root, filename)
                    process_file(file_path, header_text)
                    count += 1
                

    print(f"Headers updated in {count} files.")

# Run the script
if __name__ == "__main__":
    main()
