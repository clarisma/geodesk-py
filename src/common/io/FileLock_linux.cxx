// Copyright (c) 2023 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#include "FileLock.h"
#include <stdexcept>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

void FileLock::lock(FileHandle fileHandle, uint64_t offset, uint64_t length, bool shared)
{
    struct flock fl;
    fl.l_type = shared ? F_RDLCK : F_WRLCK;
    fl.l_whence = SEEK_SET;
    fl.l_start = offset;
    fl.l_len = length;

    // F_SETLKW waits indefinitely for lock
    if (fcntl(fileHandle, F_SETLKW, &fl) == -1)
    {
        IOException::checkAndThrow();
    }
    fileHandle_ = fileHandle;
    lockOffset_ = offset;
    lockLength_ = length;
}


bool FileLock::tryLock(FileHandle fileHandle, uint64_t offset, uint64_t length, bool shared)
{
    struct flock fl;
    fl.l_type = shared ? F_RDLCK : F_WRLCK;
    fl.l_whence = SEEK_SET;
    fl.l_start = offset;
    fl.l_len = length;

    if (fcntl(fileHandle, F_SETLK, &fl) == -1)
    {
        if (errno == EACCES || errno == EAGAIN) return false;
        IOException::checkAndThrow();
    }
    fileHandle_ = fileHandle;
    lockOffset_ = offset;
    lockLength_ = length;
    return true;
}


void FileLock::release()
{
    struct flock fl;
    fl.l_type = F_UNLCK;
    fl.l_whence = SEEK_SET;
    fl.l_start = lockOffset_;
    fl.l_len = lockLength_;

    if (fcntl(fileHandle_, F_SETLK, &fl) == -1)
    {
        IOException::checkAndThrow();
    }
    fileHandle_ = -1;
    lockLength_ = 0;
}
