option(BUILD_WHEELS "Build wheels via cibuildwheel" OFF)

cmake_minimum_required (VERSION 3.14)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_OSX_DEPLOYMENT_TARGET "10.15" CACHE STRING "Minimum OS X deployment version")
 
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "Building GeoDesk for Python")

# Unbloat executable via section garbage collection
if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_C_COMPILER_ID MATCHES "Clang")
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--gc-sections")
	# target_link_options(geodesk-py-test PRIVATE -Wl,--gc-sections)
endif()

project ("geodesk-py")

set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "Build shared libraries" FORCE)

# TODO: Make config more flexible
#if(MSVC)
#    add_compile_options(/arch:AVX2)
#endif()

# Set the CMAKE_CXX_FLAGS_RELWITHDEBINFO variable to remove the NDEBUG definition
# This approach ensures that assertions are enabled in RelWithDebInfo configuration
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} -U NDEBUG")

include(FetchContent)

# Fetch GEOS 3.11.3
FetchContent_Declare(geos
    URL https://download.osgeo.org/geos/geos-3.11.3.tar.bz2
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE)
FetchContent_MakeAvailable(geos)

if(TARGET geos AND TARGET geos_c)
	message(STATUS "GEOS and GEOS C API are available.")
else()
	message(FATAL_ERROR "GEOS or GEOS C API target not found.")
endif()


if(BUILD_WHEELS)
	find_package(PythonExtensions REQUIRED)
else()
	 find_package(Python REQUIRED COMPONENTS Interpreter Development)
	 include_directories(${Python_INCLUDE_DIRS})

	if (Python_FOUND)
		message(STATUS "Python found: ${Python_EXECUTABLE}")
		message(STATUS "Python version: ${Python_VERSION}")
		message(STATUS "Python libraries: ${Python_LIBRARIES}")
		message(STATUS "Python includes: ${Python_INCLUDE_DIRS}")
	else()
		message(FATAL_ERROR "Python not found!")
	endif()
endif()

# Pass Python variables to submodules
set(Python_INCLUDE_DIRS ${Python_INCLUDE_DIRS} CACHE INTERNAL "Python include directories")
set(Python_LIBRARIES ${Python_LIBRARIES} CACHE INTERNAL "Python libraries")


set(GEODESK_PYTHON ON CACHE INTERNAL "Enable GeoDesk Python support")
if(BUILD_WHEELS)
	set(GEODESK_PYTHON_WHEELS ON CACHE INTERNAL "Enable Support for Python Wheels")
endif()

# Set the option to enable GEOS support for libgeodesk
set(GEODESK_WITH_GEOS ON CACHE INTERNAL "Enable GEOS support in libgeodesk")
set(GEOS_INCLUDE_PATHS "${geos_SOURCE_DIR}/include" "${geos_BINARY_DIR}/capi" CACHE INTERNAL "")
set(GEODESK_EXTRA_INCLUDE_DIRS "${CMAKE_SOURCE_DIR}/src")
# Include the libgeodesk subdirectory
add_subdirectory(lib/libgeodesk)

file(GLOB_RECURSE SOURCE_FILES "src/*.cpp")
include_directories(src lib/libgeodesk/include ${GEOS_INCLUDE_PATHS})

set(LIBRARIES
	geodesk
	${Python_LIBRARIES}
	geos_c
	geos
)

if(NOT BUILD_WHEELS)
	# Create test harness
	# message(STATUS "INCLUDES = ${INCLUDES}")
	add_executable (geodesk-py-test "src/main.cxx" ${SOURCE_FILES})
	# target_include_directories(geodesk-py PRIVATE ${INCLUDES})
	target_link_libraries(geodesk-py-test PRIVATE ${LIBRARIES})
	target_compile_definitions(geodesk-py-test PRIVATE GEODESK_PYTHON)
	target_compile_definitions(geodesk-py-test PRIVATE GEODESK_WITH_GEOS)
	target_compile_definitions(geodesk-py-test PRIVATE _WIN32_WINNT=0x0601)
endif()

add_library(geodesk_py MODULE ${SOURCE_FILES})
set_target_properties(geodesk_py PROPERTIES OUTPUT_NAME "geodesk")
# target_include_directories(geodesk PRIVATE ${INCLUDES})
target_link_libraries(geodesk_py ${LIBRARIES})
target_compile_definitions(geodesk_py PRIVATE GEODESK_WITH_GEOS)
target_compile_definitions(geodesk_py PRIVATE INCLUDE_NLOHMANN_JSON_HPP_)
if(BUILD_WHEELS)
	python_extension_module(geodesk_py)
else()
	set_target_properties(geodesk_py PROPERTIES PREFIX "") # to remove 'lib' prefix on Unix systems
	if(WIN32)
		set_target_properties(geodesk_py PROPERTIES SUFFIX ".pyd")
	endif()
endif()

# message(STATUS "C++ Compiler Flags, Release build: ${CMAKE_CXX_FLAGS_RELEASE}")
  
target_compile_definitions(geodesk_py PRIVATE GEODESK_PYTHON)
# TODO: Add tests and install targets if needed.

if(BUILD_WHEELS)
	install(TARGETS geodesk_py
		LIBRARY DESTINATION geodesk)
else()
	install(TARGETS geodesk_py
			LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}
			RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX})
endif()
