// Copyright (c) 2024 Clarisma / GeoDesk contributors
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


void File::error(const char* what)
{

}


ByteBlock File::readAll(const char* filename)
{
    File file;
    file.open(filename, READ);
    uint64_t size = file.size();
    uint8_t* data = new uint8_t[size];
    uint64_t bytesRead = file.read(data, size);
    if (bytesRead != size)
    {
        throw IOException("%s: Expected to read %lld bytes instead of %lld",
            filename, size, bytesRead);
    }
    return ByteBlock(data, size);
}


void File::writeAll(const char* filename, const void* data, size_t size)
{
    File file;
    file.open(filename, WRITE | CREATE | REPLACE_EXISTING);
    uint64_t bytesWritten = file.write(data, size);
    if (bytesWritten != size)
    {
        throw IOException("%s: Expected to write %lld bytes instead of %lld",
            filename, size, bytesWritten);
    }
}

