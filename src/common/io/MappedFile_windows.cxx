// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#include "MappedFile.h"
#include <stdexcept>
#include <memoryapi.h>


void* MappedFile::map(uint64_t offset, uint64_t length, int mode)
{
    // printf("%s: Mapping %llu bytes at %llu...\n", fileName().c_str(), length, offset);
    DWORD protect = (mode & MappingMode::WRITE) ?
        PAGE_READWRITE : PAGE_READONLY;

    // We need to explicitly specify the maximum size to force Windows 
    // to grow the file to that size in case we're mapping beyond the
    // current file size
    uint64_t maxSize = offset + length;
    HANDLE mappingHandle = CreateFileMappingA(fileHandle_, NULL, protect, 
        static_cast<DWORD>(maxSize >> 32), static_cast<DWORD>(maxSize), NULL);
    if (!mappingHandle)
    {
        // Error creating file mapping
        IOException::checkAndThrow();
    }

    void* mappedAddress = MapViewOfFile(mappingHandle, 
        (mode & MappingMode::WRITE) ? FILE_MAP_ALL_ACCESS : FILE_MAP_READ,
        (DWORD)((offset >> 32) & 0xFFFFFFFF), (DWORD)(offset & 0xFFFFFFFF), length);
    CloseHandle(mappingHandle);

    if (!mappedAddress)
    {
        // Error mapping view of file
        IOException::checkAndThrow();
    }
    return mappedAddress;
}

void MappedFile::unmap(void* mappedAddress, uint64_t /* length */)
{
    UnmapViewOfFile(mappedAddress);
}

void MappedFile::sync(const void* address, uint64_t length)
{
    if (!FlushViewOfFile(address, length)) IOException::checkAndThrow();
    if (!FlushFileBuffers(handle())) IOException::checkAndThrow();
}

void MappedFile::prefetch(void* address, uint64_t length)
{
    WIN32_MEMORY_RANGE_ENTRY entry;
    entry.VirtualAddress = const_cast<void*>(address);
    entry.NumberOfBytes = length;
    PrefetchVirtualMemory(GetCurrentProcess(), 1, &entry, 0);
}



