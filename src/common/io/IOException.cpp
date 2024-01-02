// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#include "IOException.h"
#include <common/util/log.h>

#if defined(_WIN32) 
#include <Windows.h>

void IOException::checkAndThrow()
{
	DWORD errorCode = GetLastError();
	if (errorCode == 0) return;

    LPSTR buffer = nullptr;
    size_t size = FormatMessageA(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | 
        FORMAT_MESSAGE_IGNORE_INSERTS, NULL, errorCode, 
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), 
        (LPSTR)&buffer, 0, NULL);
    std::string message(buffer, size);
    LocalFree(buffer);
    throw IOException(message);
}

#elif defined(__linux__) || defined(__APPLE__) 

#include <errno.h>
#include <string.h>

void IOException::checkAndThrow()
{
    if (errno)
    {
        // LOG("errno = %d", errno);
        char buf[256];
        strcpy(buf, "Error while retrieving message");
        strerror_r(errno, buf, sizeof(buf));
        throw IOException(buf);
    }
}

#else
#error "Platform not supported"
#endif