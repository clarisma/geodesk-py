// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include <stdio.h>

#ifdef _DEBUG
#define LOG(fmt, ...) printf(fmt "\n", ##__VA_ARGS__)
#else
#define LOG(fmt, ...) do {} while (0)
#endif

#define FORCE_LOG(fmt, ...) printf(fmt "\n", ##__VA_ARGS__)
