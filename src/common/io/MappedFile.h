// Copyright (c) 2023 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include "File.h"

class MappedFile : public File
{
public:
    ~MappedFile()
    {
        close();
    }

    enum MappingMode
    {
        READ = 1 << 0,
        WRITE = 1 << 1,
    };

    void close();
    void* map(uint64_t offset, uint64_t length, int /* MappingMode */ mode);
    void unmap(void* address, uint64_t length);
    void prefetch(void* address, uint64_t length);

private:
#if defined(_WIN32) // Windows
    HANDLE mappingHandle_ = NULL;
#endif
};