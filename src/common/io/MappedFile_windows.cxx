// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#include "MappedFile.h"
#include <stdexcept>
#include <memoryapi.h>


void* MappedFile::map(uint64_t offset, uint64_t length, int mode)
{
    if (!mappingHandle_)
    {
        DWORD protect = (mode & MappingMode::WRITE) ?
            PAGE_READWRITE : PAGE_READONLY;
        mappingHandle_ = CreateFileMappingA(fileHandle_, NULL, protect, 0, 0, NULL);
        if (!mappingHandle_)
        {
            // Error creating file mapping
            IOException::checkAndThrow();
        }
    }

    void* mappedAddress = MapViewOfFile(mappingHandle_, 
        (mode & MappingMode::WRITE) ? FILE_MAP_ALL_ACCESS : FILE_MAP_READ,
        (DWORD)((offset >> 32) & 0xFFFFFFFF), (DWORD)(offset & 0xFFFFFFFF), length);
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

void MappedFile::close()
{
    if (mappingHandle_)
    {
        CloseHandle(mappingHandle_);
        mappingHandle_ = NULL;
    }
    File::close();
}

