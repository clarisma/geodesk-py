// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#if defined(_WIN32) 
#include "FileLock_windows.cxx"
#elif defined(__linux__) || defined(__APPLE__) 
#include "FileLock_linux.cxx"
#else
#error "Platform not supported"
#endif
