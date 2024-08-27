// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once
// #include <common/util/exeption.h>
#include <exception>
#include <string>
#include <zlib.h>

class ZipException : public std::runtime_error
{
public:
    ZipException(int zlibErrorCode) : 
        std::runtime_error(zError(zlibErrorCode)),
        zlibErrorCode_(zlibErrorCode)
    {
    }

    ZipException(const char* message) :
        std::runtime_error(message),
        zlibErrorCode_(0)
    {
    }
    
    /*
    virtual const char* what() const noexcept override 
    {
        return message_.c_str();
    }
    */

    int zlibErrorCode() const noexcept { return zlibErrorCode_; }

private:
    // std::string message_;
    int zlibErrorCode_;
};
