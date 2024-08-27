// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include <cstdint>
#include <string_view>

namespace Math
{
/**
 * Parses a double value, ignoring any additional non-numeric text.
 * Scientific notation is not supported.
 *
 * @param s		   pointer to the string
 * @param len      length of the string
 * @param pResult  where to store the parsed double
 * @returns true if successful, else false if the start of the string
 *    does not contain a valid number
 */
extern bool parseDouble(const char* s, size_t len, double* pResult);
inline bool parseDouble(std::string_view s, double* pResult)
{
	return parseDouble(s.data(), s.length(), pResult);
}

/**
 * Returns average of two int32_t values, ensuring that potential
 * overlow is handled correctly.
 */
inline int32_t avg(int32_t a, int32_t b)
{
	return static_cast<int32_t>((static_cast<int64_t>(a) + b) / 2);
}

extern double POWERS_OF_10[];
}
