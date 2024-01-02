// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include "File.h"

class FileLock
{
public:
    FileLock() : lockLength_(0)
    {
#if defined(_WIN32) || defined(_WIN64)
        fileHandle_ = INVALID_HANDLE_VALUE;
        overlapped_.hEvent = 0;
#else
        fileHandle_ = -1;
        lockOffset_ = 0;
#endif
    }

    ~FileLock()
    {
        if (isLocked()) release();
    }

    bool isLocked() const { return lockLength_ != 0; }
    void lock(FileHandle fileHandle, uint64_t offset, uint64_t length, bool shared);
    bool tryLock(FileHandle fileHandle, uint64_t offset, uint64_t length, bool shared);
    void release();

private:
    FileHandle fileHandle_;
#if defined(_WIN32) || defined(_WIN64)
    OVERLAPPED overlapped_;
#else
    uint64_t lockOffset_;
#endif
    uint64_t lockLength_;
};

