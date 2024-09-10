# TODO: Make sure to compile libgeos using option -fPIC on Linux!

# TODO: enable SIMD instructions
# (try -march=native, but don't use on GitHub's CI/CD)
# Probably safe to use -msse4.2 -mavx2 -mfma -mbmi2 
# -mpclmul -maes -mpopcnt -mlzcnt -mf16c -mmovbe   -mfsgsbase
# Available but probably don't apply (except maybe https support?)
# -mrdseed -mrdrnd -maes

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
# add_definitions(-D_WIN32_WINNT=0x0603)
add_compile_definitions(_WIN32_WINNT=0x0603)


if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build." FORCE)
endif()

message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

# Unbloat executable via section garbage collection
if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_C_COMPILER_ID MATCHES "Clang")
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--gc-sections")
	# target_link_options(geodesk-py PRIVATE -Wl,--gc-sections)
endif()

project ("geodesk-py")

set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(BUILD_SHARED_LIBS OFF)

# TODO: Make config more flexible
#if(MSVC)
#    add_compile_options(/arch:AVX2)
#endif()

add_subdirectory(third_party/geos)
add_subdirectory(third_party/zlib)
add_subdirectory(third_party/catch2)

# Set the CMAKE_CXX_FLAGS_RELWITHDEBINFO variable to remove the NDEBUG definition
# This approach ensures that assertions are enabled in RelWithDebInfo configuration
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} -U NDEBUG")

include_directories(src)
# include_directories(src include/geodesk)

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

set(GEOS_ROOT "${CMAKE_BINARY_DIR}/third_party/geos")


set(GEOS_LIBRARY_DIR "${GEOS_ROOT}/lib")

set(LIBRARIES 
	${Python_LIBRARIES}
	zlibstatic
	# asmjit
	geos_c
	geos
)

set(INCLUDES
	"${GEOS_ROOT}/include/capi" 
	"${GEOS_ROOT}/include"
	third_party/zlib
	"${CMAKE_BINARY_DIR}/third_party/zlib"
)

if(NOT BUILD_WHEELS)
	# Create test harness
	add_executable (geodesk-py "src/main.cxx" ${SOURCE_FILES})
	target_include_directories(geodesk-py PRIVATE ${INCLUDES})
	target_link_libraries(geodesk-py PRIVATE ${LIBRARIES})
	target_compile_definitions(geodesk-py PRIVATE GEODESK_PYTHON)
	target_compile_definitions(geodesk-py PRIVATE _WIN32_WINNT=0x0601)

	file(GLOB_RECURSE TEST_SOURCE_FILES "test_cpp/*.cpp")
	add_executable (geodesk-test ${GOL_SOURCE_FILES} ${TEST_SOURCE_FILES})
	target_include_directories(geodesk-test PRIVATE ${INCLUDES} third_party/catch2/src)
	target_link_libraries(geodesk-test PRIVATE ${LIBRARIES} Catch2::Catch2WithMain)
endif()

add_library(geodesk MODULE ${SOURCE_FILES})
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
  
target_compile_definitions(geodesk PRIVATE GEODESK_PYTHON)
# TODO: Add tests and install targets if needed.

if(BUILD_WHEELS)
	install(TARGETS geodesk
		LIBRARY DESTINATION geodesk)
else()
	install(TARGETS geodesk
			LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}
			RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX})
endif()
