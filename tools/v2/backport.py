import os
import shutil

def should_exclude_directory(relative_path, exclude_dirs):
    for exclude_dir in exclude_dirs:
        if relative_path == exclude_dir or relative_path.startswith(exclude_dir + os.sep):
            return True
    return False

def copy_files(src_dir, dest_dir, exclude_dirs):
    # Normalize and convert the exclude_dirs to absolute paths relative to src_dir
    exclude_dirs = [os.path.normpath(exclude_dir) for exclude_dir in exclude_dirs]

    # Walk through the source directory
    for root, dirs, files in os.walk(src_dir):
        # Calculate the relative path from the source directory
        relative_path = os.path.relpath(root, src_dir)

        # Skip directories that match the exclude patterns
        if should_exclude_directory(relative_path, exclude_dirs):
            continue

        # Determine the destination path
        dest_path = os.path.join(dest_dir, relative_path)

        # Create the destination directory if it doesn't exist
        os.makedirs(dest_path, exist_ok=True)

        # Copy the files
        for file in files:
            src_file = os.path.join(root, file)
            dest_file = os.path.join(dest_path, file)
            shutil.copy2(src_file, dest_file)
            print(f"Copied {src_file} to {dest_file}")


def delete_all_files_in_directory(directory, exclude_dirs):
    # Normalize the exclude_dirs to handle relative paths correctly
    exclude_dirs = [os.path.normpath(exclude_dir) for exclude_dir in exclude_dirs]

    # Walk through the directory
    for root, dirs, files in os.walk(directory):
        # Calculate the relative path from the root directory
        relative_path = os.path.relpath(root, directory)

        # Skip directories that match the exclude patterns
        if should_exclude_directory(relative_path, exclude_dirs):
            dirs[:] = []  # Do not descend into this directory
            continue

        # Delete the files in the current directory
        for file in files:
            file_path = os.path.join(root, file)
            try:
                os.remove(file_path)
                print(f"Deleted file: {file_path}")
            except Exception as e:
                print(f"Failed to delete {file_path}: {e}")

source_directory = "C:/dev/geodesk-py2"
destination_directory = "C:/dev/geodesk-py"

excluded_directories = [
    ".git",
    ".idea",
    ".pytest_cache",
    ".vs",
    "cmake-build-debug",
    "out",
    "src/build",
    "src/gol",
    "src/osm",
    "src/tile",
    "third_party",
    "tools/v2",
    "venv"
]

delete_all_files_in_directory(destination_directory, [ ".git", "third_party" ] )
copy_files(source_directory, destination_directory, excluded_directories)
