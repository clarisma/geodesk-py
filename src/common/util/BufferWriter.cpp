// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#include "BufferWriter.h"
#include <cassert>
#include <cstdio>
#include <math.h>
#include <common/util/log.h>
#include <common/util/math.h>

// TODO: have to distinguish between "flushing to end" and "flushing because full"



void BufferWriter::formatDouble(double d, int precision, bool zeroFill)
{
	assert(precision >= 0 && precision <= 15);
	char buf[64];
	double multiplier = Math::POWERS_OF_10[precision];
	long long roundedScaled = static_cast<long long>(round(d * multiplier));
	long long intPart = static_cast<long long>(roundedScaled / multiplier);
	unsigned long long fracPart = static_cast<unsigned long long>(
		abs(roundedScaled - intPart * multiplier));
	char* end = buf + sizeof(buf);
	char* start = formatFractionalReverse(fracPart, &end, precision, zeroFill);
	if (start != end) *(--start) = '.';
	start = formatLongReverse(intPart, start, d < 0);
	writeBytes(start, end - start);
}

void BufferWriter::formatInt(int64_t d)
{
	char buf[32];
	char* end = buf + sizeof(buf);
	char* start = formatLongReverse(d, end, d<0);
	writeBytes(start, end - start);
}

void BufferWriter::writeReplacedString(const char* s, const char* find, size_t findLen,
	const char* replaceWith, size_t replaceLen)
{
	for (;;)
	{
		const char* next = strstr(s, find);
		if (!next)
		{
			writeString(s);
			return;
		}
		writeBytes(s, next - s);
		writeBytes(replaceWith, replaceLen);
		s = next + findLen;
	}
}

void BufferWriter::writeJsonEscapedString(const char* s, size_t len)
{
	// LOG("Escaping \"%.*s\"", len, s);
	const char* end = s + len;
	while (s < end)
	{
		unsigned char ch = static_cast<unsigned char>(*s++);
		if (ch == '\"' || ch == '\\')
		{
			writeByte('\\');
		}
		else if (ch < ' ')
		{
			switch (ch)
			{
			case '\b':
				ch = 'b';
				break;
			case '\f':
				ch = 'f';
				break;
			case '\n':
				ch = 'n';
				break;
			case '\r':
				ch = 'r';
				break;
			case '\t':
				ch = 't';
				break;
			default:
				char buf[10];
				sprintf(buf, "\\u%04X", ch);
				writeBytes(buf, 6);
				continue;
			}
			writeByte('\\');
		}
		writeByte(ch);
	}
}

