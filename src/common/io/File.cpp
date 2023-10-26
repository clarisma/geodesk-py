// Copyright (c) 2023 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#if defined(_WIN32) 
#include "File_windows.cxx"
#elif defined(__linux__) || defined(__APPLE__) 
#include "File_linux.cxx"
#else
#error "Platform not supported"
#endif

const char* File::extension(const char* filename, size_t len)
{
	const char* p = filename + len - 1;
	while (p > filename && *p != '.' && *p != '/' && *p != '\\')
	{
		p--;
	}
	return *p == '.' ? p : "";
}
