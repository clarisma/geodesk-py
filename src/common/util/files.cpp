// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#include "files.h"
#include <fstream>
#include <iterator>
#include <vector>

char* readFile(const char *fileName) 
{
	// Open file for binary reading
	std::ifstream file(fileName, std::ios::binary);

	// Move file position to the end to get the file size
	file.seekg(0, std::ios::end);
	std::streamsize size = file.tellg();
	file.seekg(0, std::ios::beg);

	char* data = new char[size];
	file.read(data, size);

	return data;
}
