// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once
#include <exception>
#include <string>
#include <zlib.h>

class ZipException : public std::exception 
{
public:
    ZipException(int zlibErrorCode) : 
        zlibErrorCode_(zlibErrorCode),
        message_(zError(zlibErrorCode))
    {
    }

    virtual const char* what() const noexcept override 
    {
        return message_.c_str();
    }

    int zlibErrorCode() const noexcept { return zlibErrorCode_; }

private:
    std::string message_;
    int zlibErrorCode_;
};
