// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include <cstring>

class Buffer;

int compare_strings(const char* s1, int len1, const char* s2, int len2);
void replace_all(Buffer& buf, const char* s, const char* find, size_t findLen, const char* replaceWith);


