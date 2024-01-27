# TODO: Make sure to compile libgeos using option -fPIC on Linux!

option(BUILD_WHEELS "Build wheels via cibuildwheel" OFF)

cmake_minimum_required (VERSION 3.8)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_OSX_DEPLOYMENT_TARGET "10.15" CACHE STRING "Minimum OS X deployment version")

# Used for testing, disable in release!
# add_definitions(-DGEODESK_TEST_PERFORMANCE)

# Define _WIN32_WINNT to be at least Windows 8.1
# (Needed for virtual memory management)
add_definitions(-D_WIN32_WINNT=0x0603)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build." FORCE)
endif()

message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project ("geodesk-py")

set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(BUILD_SHARED_LIBS OFF)
add_subdirectory(third_party/geos)
add_subdirectory(third_party/zlib)

include_directories(src)

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

# AsmJIT (not needed for now)
# include_directories(third-party/asmjit/src)
# link_directories(third-party/asmjit/out/build/x64-Release)

file(GLOB_RECURSE SOURCE_FILES "src/*.cpp")
list(REMOVE_ITEM SOURCE_FILES "src/main.cpp")
list(REMOVE_ITEM SOURCE_FILES "src/gol/main.cpp")

# set(BOOST_ROOT "third-party/boost_1_81_0")
# set(Boost_USE_STATIC_LIBS ON)  # Use static libraries. Switch to OFF if you want to use dynamic
# set(Boost_USE_MULTITHREADED ON)
# set(Boost_USE_STATIC_RUNTIME OFF)  # Adjust as per your needs
# set(BOOST_THREAD_USE_LIB OFF)
# find_package(Boost 1.81.0 REQUIRED COMPONENTS thread iostreams)  
# find_package(Boost 1.81.0 REQUIRED)  
# include_directories(${Boost_INCLUDE_DIRS})

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
	set(GEOS_ROOT "${CMAKE_SOURCE_DIR}/out/build/x64-debug/geos")
else()
	set(GEOS_ROOT "${CMAKE_SOURCE_DIR}/out/build/x64-release/geos")
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
	set(ZLIB_ROOT "${CMAKE_SOURCE_DIR}/out/build/x64-debug/third_party/zlib")
else()
	set(ZLIB_ROOT "${CMAKE_SOURCE_DIR}/out/build/x64-release/third_party/zlib")
endif()


set(GEOS_LIBRARY_DIR "${GEOS_ROOT}/lib")

set(LIBRARIES 
	${Python_LIBRARIES}
	# ${Boost_LIBRARIES}
	zlibstatic
	# asmjit
	geos_c
	geos
)

set(INCLUDES
	"${GEOS_ROOT}/include/capi" 
	"${GEOS_ROOT}/include"
	third_party/zlib
	${ZLIB_ROOT}
)

# Conditionally add Ws2_32 for Windows builds (required for ntohl in OsmPbfReader)
if(WIN32)
    list(APPEND LIBRARIES Ws2_32)
endif()

if(NOT BUILD_WHEELS)
	# Create test harness
	add_executable (geodesk-py "src/main.cxx" ${SOURCE_FILES})
	target_include_directories(geodesk-py PRIVATE ${INCLUDES})
	target_link_libraries(geodesk-py PRIVATE ${LIBRARIES})
	target_compile_definitions(geodesk-py PRIVATE GEODESK_PYTHON=1)
	target_compile_definitions(geodesk-py PRIVATE _WIN32_WINNT=0x0601)

	set(GOL_SOURCE_FILES ${SOURCE_FILES})
	list(FILTER GOL_SOURCE_FILES EXCLUDE REGEX "src/python/.*")
	add_executable (gol "src/gol/main.cxx" ${GOL_SOURCE_FILES})
	target_include_directories(gol PRIVATE ${INCLUDES})
	target_link_libraries(gol PRIVATE ${LIBRARIES})
	target_compile_definitions(gol PRIVATE _WIN32_WINNT=0x0601)
endif()

add_library(geodesk MODULE ${SOURCE_FILES} )
target_include_directories(geodesk PRIVATE ${INCLUDES})
target_link_libraries(geodesk ${LIBRARIES})
if(BUILD_WHEELS)
	python_extension_module(geodesk)
else()
	set_target_properties(geodesk PROPERTIES PREFIX "") # to remove 'lib' prefix on Unix systems
	if(WIN32)
		set_target_properties(geodesk PROPERTIES SUFFIX ".pyd")
	endif()
endif()

# message(STATUS "C++ Compiler Flags, Release build: ${CMAKE_CXX_FLAGS_RELEASE}")

target_compile_definitions(geodesk PRIVATE GEODESK_PYTHON=1)
target_compile_definitions(geodesk PRIVATE _WIN32_WINNT=0x0601)
# TODO: Add tests and install targets if needed.

if(BUILD_WHEELS)
	install(TARGETS geodesk
		LIBRARY DESTINATION geodesk)
else()
	install(TARGETS geodesk
			LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}
			RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX})
endif()
