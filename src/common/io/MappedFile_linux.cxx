// Copyright (c) 2023 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#include "MappedFile.h"
#include <stdexcept>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>


void* MappedFile::map(uint64_t offset, uint64_t length, int mode)
{
    int prot = (mode & MappingMode::WRITE) ? (PROT_READ | PROT_WRITE) : PROT_READ;
    void* mappedAddress = mmap(nullptr, length, prot, MAP_SHARED, fileHandle_, offset);
    if (mappedAddress == MAP_FAILED)
    {
        // Error mapping file
        IOException::checkAndThrow();
    }
    return mappedAddress;
}

void MappedFile::unmap(void* address, uint64_t length)
{
    munmap(address, length);
}


void MappedFile::close()
{
    File::close();
}
