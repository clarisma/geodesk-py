// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#include "strings.h"
#include "Buffer.h"
#include "BufferWriter.h"
#include <algorithm>
#include <cstring>

int compare_strings(const char* s1, int len1, const char* s2, int len2)
{
    int res = std::strncmp(s1, s2, std::min(len1, len2));
    if (res != 0) return res;
    return len1 - len2;
}

/*
void replace_all(Buffer& buf, const char* s, const char* find, size_t findLen, const char* replaceWith)
{
    BufferWriter out(&buf);
    for (;;)
    {
        const char* next = strstr(s, find);
        if (!next)
        {
            out.writeString(s);
            out.flush();
            return;
        }
    }
}

*/

