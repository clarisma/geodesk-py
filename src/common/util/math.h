// Copyright (c) 2023 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

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

	extern double POWERS_OF_10[];
}
