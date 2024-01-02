// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#include "FileLock.h"
#include <stdexcept>

#include <windows.h>

void FileLock::lock(FileHandle fileHandle, uint64_t offset, uint64_t length, bool shared)
{
    overlapped_.Offset = offset & 0xFFFFFFFF;
    overlapped_.OffsetHigh = offset >> 32;

    if (!LockFileEx(fileHandle, shared ? 0 : LOCKFILE_EXCLUSIVE_LOCK, 0, length & 0xFFFFFFFF, length >> 32, &overlapped_))
    {
        IOException::checkAndThrow();
    }
    fileHandle_ = fileHandle;
    lockLength_ = length;
}

bool FileLock::tryLock(FileHandle fileHandle, uint64_t offset, uint64_t length, bool shared)
{
    overlapped_.Offset = offset & 0xFFFFFFFF;
    overlapped_.OffsetHigh = offset >> 32;
    DWORD lockFlags = shared ? LOCKFILE_FAIL_IMMEDIATELY : LOCKFILE_EXCLUSIVE_LOCK | LOCKFILE_FAIL_IMMEDIATELY;
    if (!LockFileEx(fileHandle, lockFlags, 0, length & 0xFFFFFFFF, length >> 32, &overlapped_))
    {
        // TODO: distinguish failure to get lock from other errors
        return false;
    }
    fileHandle_ = fileHandle;
    lockLength_ = length;
    return true;
}

void FileLock::release()
{
    if (!UnlockFileEx(fileHandle_, 0, lockLength_ & 0xFFFFFFFF, lockLength_ >> 32, &overlapped_))
    {
        IOException::checkAndThrow();
    }
    fileHandle_ = INVALID_HANDLE_VALUE;
    lockLength_ = 0;
}
