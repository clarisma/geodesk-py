// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#include "File.h"
#include <stdexcept>


void File::open(const char* filename, int mode)
{
    DWORD access = 0;
    DWORD creationDisposition = 0;

    if (mode & OpenMode::READ)
    {
        access |= GENERIC_READ;
    }
    if (mode & OpenMode::WRITE)
    {
        access |= GENERIC_WRITE;
    }

    if (mode & OpenMode::CREATE)
    {
        creationDisposition = OPEN_ALWAYS;
    }
    else
    {
        creationDisposition = OPEN_EXISTING;
    }

    // TODO: set a share mode instead of `0`
    //  Should use (FILE_SHARE_READ | FILE_SHARE_WRITE) unless file is 
    //  expressively opened as exclusive? or other way around, make shared access explicit?
    //  (Linux does not have exclusive access mode, multiple processes can read/write)
    // For now, we enable shared access implicitly to match Linux behavior
    fileHandle_ = CreateFileA(filename, access, FILE_SHARE_READ | FILE_SHARE_WRITE, 
        NULL, creationDisposition, FILE_ATTRIBUTE_NORMAL, NULL);

    if (fileHandle_ == INVALID_HANDLE_VALUE)
    {
        if (GetLastError() == ERROR_FILE_NOT_FOUND)
        {
            throw FileNotFoundException(filename);
        }
        IOException::checkAndThrow();
    }

    // TODO: only do this if file did not exist
    if (mode & SPARSE)
    {
        DWORD bytesReturned;
        DeviceIoControl(fileHandle_, FSCTL_SET_SPARSE, NULL, 0, NULL, 0, &bytesReturned, NULL);
    }
}

void File::close()
{
    if (fileHandle_ != INVALID_HANDLE_VALUE)
    {
        CloseHandle(fileHandle_);
        fileHandle_ = INVALID_HANDLE_VALUE;
    }
}


uint64_t File::size() const
{
    LARGE_INTEGER fileSize;
    if (!GetFileSizeEx(fileHandle_, &fileSize))
    {
        IOException::checkAndThrow();
    }
    return fileSize.QuadPart;
}

void File::setSize(uint64_t newSize)
{
    LARGE_INTEGER newPos;
    newPos.QuadPart = newSize;
    if (!SetFilePointerEx(fileHandle_, newPos, NULL, FILE_BEGIN))
    {
        IOException::checkAndThrow();
    }
    if (!SetEndOfFile(fileHandle_))
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
    if (!FlushFileBuffers(fileHandle_)) 
    {
        IOException::checkAndThrow();
    }
}


void File::seek(uint64_t posAbsolute)
{
    DWORD dwPtrLow = posAbsolute & 0xFFFFFFFF;
    LONG  dwPtrHigh = (posAbsolute >> 32) & 0xFFFFFFFF;
    DWORD resultLow = SetFilePointer(fileHandle_, dwPtrLow, &dwPtrHigh, FILE_BEGIN);
    if (resultLow == INVALID_SET_FILE_POINTER) // && GetLastError() != NO_ERROR) 
    {
        IOException::checkAndThrow();
    }
}


size_t File::read(void* buf, size_t length)
{
    DWORD bytesRead;
    if (!ReadFile(fileHandle_, buf, length, &bytesRead, NULL))
    {
        IOException::checkAndThrow();
    }
    return bytesRead;
}


size_t File::write(const void* buf, size_t length)
{
    DWORD bytesWritten;
    if (!WriteFile(fileHandle_, buf, length, &bytesWritten, NULL))
    {
        IOException::checkAndThrow();
    }
    return bytesWritten;
}
