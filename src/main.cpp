// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

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

	std::filesystem::current_path("c:\\dev\\geodesk-py\\test");

	// char* script = readFile(argc > 1 ? argv[1] : "script.py");

	// std::cout << "Test!\n";
    // Py_SetPath(L"C:\\python\\Lib;C:\\python\\Lib\\site-packages");
	Py_SetPath(L"C:\\Python\\python311.zip;C:\\Python\\DLLs;C:\\Python\\Lib;C:\\Python;C:\\Python\\Lib\\site-packages");
    if(PyImport_AppendInittab("geodesk", &PyInit_geodesk) < 0)
    {
        std::cout << "PyImport_AppendInittab failed.\n";
    }
    Py_Initialize();
	auto start = std::chrono::high_resolution_clock::now();

	// const char* script = argc > 1 ? argv[1] : "c:\\dev\\geodesk-py\\query.py";
	const char* script = argc > 1 ? argv[1] : "c:\\dev\\geodesk-py\\test\\test_main.py";
	FILE* file = fopen(script, "r");
	PyRun_SimpleFile(file, "query.py");
	fclose(file);

	/*
    PyRun_SimpleString(
        "from geodesk import *\n"
        "\n"
        "box = Box(100,200,300,400)\n"
        "print(box)\n"
		// "world = Features('c:\\\\geodesk\\\\tests\\\\monaco.gol')\n"
		"world = Features('c:\\\\geodesk\\\\tests\\\\de.gol')\n"
		// "print(world.count)\n"
		"streets = world('w[highway=residential]')\n"
		// "for s in streets:\n"
		// "    print (s.highway)\n"
		"print(streets.count)\n"
		"for s in streets:\n"
		"    for k,v in s.tags:\n"
		"        print (f\"{k} = {v}\")\n"
		
		"count = 0\n"
		"for f in world:\n"
		"    v = f['name']\n"
		"    if v is not None:\n"
		"        print(v)\n"
		"    count += 1\n"
		"print(count)\n"
		
		"part = world(box)\n"
		// "bad = world(2)\n"
		// "bad = Features('c:\\\\notfound.bad')\n"
    );
	*/
	auto stop = std::chrono::high_resolution_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);

	// Print the elapsed time
	std::cout << "Time to complete query: " << duration.count() << " ms" << std::endl;

    Py_Finalize();
    return 0;
}

