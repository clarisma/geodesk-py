// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#include <stdexcept>
#include <string>

class IOException : public std::runtime_error 
{
public:
    explicit IOException(const char* message)
        : std::runtime_error(message) {}

    explicit IOException(const std::string& message)
        : std::runtime_error(message) {}

    // static void getError(char* buf);

    /**
     * On Linux, this function must only be called if the caller
     * is certain that an error occurred (errno is set) -- typically,
     * because a system call returned -1.
     */
    static void checkAndThrow();
    static void alwaysThrow();
    // static void alwaysThrow(const char* msg);
};


class FileNotFoundException : public IOException
{
public:
    explicit FileNotFoundException(const char* filename)
        : IOException(std::string(filename) + ": File not found") {}
    explicit FileNotFoundException(std::string filename)
        : FileNotFoundException(filename.c_str()) {}
};


