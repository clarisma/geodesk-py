from skbuild import setup
# from setuptools import find_packages
from pathlib import Path

# read the contents of your README file
this_directory = Path(__file__).parent
long_description = (this_directory / "README.md").read_text()

setup(
    name="geodesk",
    # version="0.0.6",  # already defined in pyproject.toml
    description='Python port of GeoDesk, a fast and storage-efficient spatial database engine for OpenStreetMap features',
    long_description=long_description,
    long_description_content_type='text/markdown',
    author="Clarisma / GeoDesk contributors",
    license="LGPL-3.0-only",  
    packages=["geodesk"],  # If you have Python packages as well
    cmake_args=['-DCMAKE_BUILD_TYPE=Release', '-DBUILD_WHEELS:BOOL=ON'],  # or Debug, or any other args you need
    # install_requires=[  # If you have Python dependencies
    #     "somepythonpackage>=1.0",
    # ],
    # install_requires=['shapely>=2.1,<2.2'], # already defined in pyproject.toml
    # include_package_data=True,
    # exclude_package_data={ "": ["*h.", "*.a", "*.lib"]},
    cmake_process_manifest_hook = \
        lambda fl: [f for f in fl if f.endswith('.pyd') or f.endswith('.so')]
        # This gets rid of the headers and libraries built by GEOS as
        # a sub-project, and only includes the actual GeoDesk Python module
)
