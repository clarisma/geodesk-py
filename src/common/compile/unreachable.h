// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#ifdef _DEBUG
#define UNREACHABLE_CASE assert(false); break;
#else
#ifdef __GNUC__  // GCC compiler check
#define UNREACHABLE_CASE __builtin_unreachable();
#elif defined(_MSC_VER)  // MSVC compiler check
#define UNREACHABLE_CASE __assume(0);
#endif
#endif
