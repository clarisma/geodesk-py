// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

static_assert(sizeof(void*) == 8, "Only 64-bit architectures are supported");

#ifndef GEODESK_PYTHON
#error "Relies on Python; enable GEODESK_PYTHON"
#endif

#include "python/module.h"
#include <iostream>
#include <chrono>
// #include "io/OsmPbfReader.h"
#include <common/util/files.h>
#include <filesystem>

/*
void testOsmPbfReader()
{
	std::cout << "Testing OsmPbfReader...\n";
	auto start = std::chrono::high_resolution_clock::now();
	OsmPbfReader reader(4, 16);
	reader.read("c:\\geodesk\\mapdata\\de-2022-11-28.osm.pbf");
	auto stop = std::chrono::high_resolution_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
	std::cout << "Time to read osm.pbf: " << duration.count() << " microsecs" << std::endl;
}
*/

void alloc_test()
{
	std::cout << "Starting new/delete test...\n";

	const int count = 10'000'000;

	auto start = std::chrono::high_resolution_clock::now();

	void** ptrs = new void* [count];
	for (int i = 0; i < count; i++)
	{
		ptrs[i] = new char[20];
	}

	for (int i = 0; i < count; i++)
	{
		delete ptrs[i];
	}


	// Stop the timer
	auto stop = std::chrono::high_resolution_clock::now();

	// Calculate the elapsed time
	auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);

	// Print the elapsed time
	std::cout << "Time to alloc/free: " << duration.count() << " microsecs" << std::endl;
}


void pyalloc_test()
{
	std::cout << "Starting pyalloc test...\n";

	const int count = 10'000'000;

	auto start = std::chrono::high_resolution_clock::now();

	void** ptrs = new void* [count];
	for (int i = 0; i < count; i++)
	{
		ptrs[i] = PyMem_Malloc(20);
	}

	for (int i = 0; i < count; i++)
	{
		PyMem_Free(ptrs[i]);
	}


	// Stop the timer
	auto stop = std::chrono::high_resolution_clock::now();

	// Calculate the elapsed time
	auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);

	// Print the elapsed time
	std::cout << "Time to alloc/free: " << duration.count() << " microsecs" << std::endl;
}


void pyobjalloc_test()
{
	std::cout << "Starting pyobjalloc test...\n";

	const int count = 10'000'000;

	auto start = std::chrono::high_resolution_clock::now();

	void** ptrs = new void* [count];
	for (int i = 0; i < count; i++)
	{
		ptrs[i] = PyObject_Malloc(20);
	}

	for (int i = 0; i < count; i++)
	{
		PyObject_Free(ptrs[i]);
	}


	// Stop the timer
	auto stop = std::chrono::high_resolution_clock::now();

	// Calculate the elapsed time
	auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);

	// Print the elapsed time
	std::cout << "Time to alloc/free: " << duration.count() << " microsecs" << std::endl;
}


int main(int argc, char* argv[]) 
{
	/*
	alloc_test();
	pyalloc_test();
	pyobjalloc_test();
	testOsmPbfReader();
	return 0;
	*/

	std::filesystem::path currentPath = std::filesystem::current_path();
	std::cout << "Current Working Directory: " << currentPath.string() << std::endl;

	// Py_SetPath(L"C:\\python\\Lib;C:\\python\\Lib\\site-packages");
#ifdef _WIN32
    Py_SetPath(L"C:\\Python\\python311.zip;C:\\Python\\DLLs;C:\\Python\\Lib;C:\\Python;C:\\Python\\Lib\\site-packages");
#endif
    if(PyImport_AppendInittab("geodesk", &PyInit_geodesk) < 0)
    {
        std::cout << "PyImport_AppendInittab failed.\n";
    }
    Py_Initialize();
	auto start = std::chrono::high_resolution_clock::now();

	const char* script = argc > 1 ? argv[1] : "test_main.py";

	FILE* file = fopen(script, "r");
	PyRun_SimpleFile(file, script);
	fclose(file);

	auto stop = std::chrono::high_resolution_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);

	// Print the elapsed time
	std::cout << "Time to complete query: " << duration.count() << " ms" << std::endl;

    Py_Finalize();
    return 0;
}

