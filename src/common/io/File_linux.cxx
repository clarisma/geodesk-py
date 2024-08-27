// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#include "File.h"
#include <stdexcept>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h> // for off_t
#include <unistd.h>
#include <fcntl.h>


void File::open(const char* filename, int mode) 
{
    int flags = 0;

    if ((mode & OpenMode::READ) && (mode & OpenMode::WRITE))
    {
        flags |= O_RDWR;
    }
    else if (mode & OpenMode::READ)
    {
        flags |= O_RDONLY;
    }
    else if (mode & OpenMode::WRITE)
    {
        flags |= O_WRONLY;
    }
    
    if (mode & REPLACE_EXISTING)
    {
        flags |= O_TRUNC;
    }
    if (mode & OpenMode::CREATE)
    {
        flags |= O_CREAT;
    }

    fileHandle_ = ::open(filename, flags, 0666);

    if (fileHandle_ == -1)
    {
        if(errno == ENOENT) throw FileNotFoundException(filename);
        IOException::checkAndThrow();
    }

    // Sparse files are inherently supported on most UNIX filesystems. 
    // You don't need to specify a special flag, just don't write to all parts of the file.
}

void File::close()
{
    if (fileHandle_ != -1)
    {
        ::close(fileHandle_);
        fileHandle_ = -1;
    }
}



uint64_t File::size() const
{
    struct stat fileInfo;
    if (fstat(fileHandle_, &fileInfo) != 0)
    {
        IOException::checkAndThrow();
    }
    return fileInfo.st_size;
}

void File::setSize(uint64_t newSize)
{
    if (ftruncate(fileHandle_, newSize) != 0)
    {
        IOException::checkAndThrow();
    }
}

void File::expand(uint64_t newSize)
{
    if (size() < newSize)
    {
        setSize(newSize);
    }
}

void File::truncate(uint64_t newSize)
{
    if (size() > newSize)
    {
        setSize(newSize);
    }
}


void File::force()
{
    if (fsync(fileHandle_) != 0)
    {
        IOException::checkAndThrow();
    }
}


void File::seek(uint64_t posAbsolute)
{
#if defined(__APPLE__) 
    if (lseek(fileHandle_, static_cast<off_t>(posAbsolute), SEEK_SET) == -1)
#else
    if (lseek64(fileHandle_, posAbsolute, SEEK_SET) == -1)
#endif
    {
        IOException::checkAndThrow();
    }
}


size_t File::read(void* buf, size_t length)
{
    ssize_t bytesRead = ::read(fileHandle_, buf, length);
    if (bytesRead < 0)
    {
        IOException::checkAndThrow();
    }
    return bytesRead;
}

size_t File::read(uint64_t ofs, void* buf, size_t length)
{
    ssize_t bytesRead = pread(fileHandle_, buf, length, ofs);
    if (bytesRead < 0)
    {
        IOException::checkAndThrow();
    }
    return bytesRead;
}


size_t File::write(const void* buf, size_t length)
{
    ssize_t bytesWritten = ::write(fileHandle_, buf, length);
    if (bytesWritten < 0)
    {
        IOException::checkAndThrow();
    }
    return bytesWritten;
}


std::string File::fileName() const
{
    char fdPath[1024];
    char filePath[1024];
    snprintf(fdPath, sizeof(fdPath), "/proc/self/fd/%d", fileHandle_);
    ssize_t len = readlink(fdPath, filePath, sizeof(filePath) - 1);
    if (len != -1) 
    {
        filePath[len] = '\0'; // Null-terminate the result
        return std::string(filePath);
    }
    else 
    {
        return "<invalid file>";
    }
}


void File::allocate(uint64_t ofs, size_t length)
{
#ifdef __APPLE__
    // TODO: no native implementation of fallocate() on MacOS,
    //  do nothing for now
#else
    // Could use posix_fallocate on Linux as well, buf fallocate is more efficient
    if (fallocate(fileHandle_, 0, ofs, length) != 0)
    {
        IOException::checkAndThrow();
    }
#endif
}