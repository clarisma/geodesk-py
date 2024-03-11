// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#include "FileSystem.h"
#include <iostream>
#include "IOException.h"

size_t FileSystem::getBlockSize(const char* path)
{
    struct statvfs buf;
    if (statvfs(path, &buf) != 0) 
    {
        IOException::checkAndThrow(); 
        return 0;
    }
    return buf.f_bsize;
}


size_t FileSystem::getAvailableDiskSpace(const char* path)
{
    struct statvfs buf;
    if (statvfs(path.c_str(), &buf) != 0) 
    {
        IOException::checkAndThrow();
    }
    return static_cast<size_t>(buf.f_bsize) * buf.f_bavail;
}

