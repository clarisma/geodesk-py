// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#include "FileSystem.h"
#include <windows.h>
#include "IOException.h"

size_t FileSystem::getBlockSize(const char* path)
{
    DWORD sectorsPerCluster, bytesPerSector, numberOfFreeClusters, totalNumberOfClusters;

    if (!GetDiskFreeSpaceA(path, &sectorsPerCluster, &bytesPerSector, &numberOfFreeClusters, &totalNumberOfClusters)) 
    {
        IOException::checkAndThrow();
        return 0;
    }
    return sectorsPerCluster * bytesPerSector;
}


size_t FileSystem::getAvailableDiskSpace(const char* path)
{
    ULARGE_INTEGER freeBytesAvailable;
    if (!GetDiskFreeSpaceExA(path, &freeBytesAvailable, nullptr, nullptr)) 
    {
        IOException::checkAndThrow();
    }
    return freeBytesAvailable.QuadPart;
}
