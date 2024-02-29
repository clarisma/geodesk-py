// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once
#include <cstdint>

class ProgressBar
{
public:
	void progress(uint64_t units);

	static char* draw(char* p, int percentage);

private:
	static const char* BLOCK_CHARS_UTF8;

	void report();

	template <size_t N>
	static char *putString(char* p, const char(&s)[N])
	{
		memcpy(p, s, N);
		return p + N;
	}

	uint64_t totalUnits_;
	uint64_t unitsCompleted_;
};
