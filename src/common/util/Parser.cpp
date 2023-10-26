// Copyright (c) 2023 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#include "Parser.h"

#include <cstdlib>
#include <limits>
#include <cstdarg>
#include <common/text/Highlighter.h>


void Parser::error(const char* format, ...)
{
	char buf[256];
	va_list args;
	va_start(args, format);
	vsnprintf(buf, sizeof(buf), format, args);
	va_end(args);
	StringBuilder sb;
	sb.writeString(buf);
	sb.writeByte('\n');
	Highlighter::highlight(sb, pStart_, pNext_ - pStart_, 1, 31);
	throw ParseException(sb.toString());
}

void Parser::expect(char ch)
{
	if (accept(ch)) return;
	error("Expected %c", ch);
}

void Parser::skipWhitespace()
{
	for (;;)
	{
		unsigned char ch = static_cast<unsigned char>(*pNext_) - 1;
		if (ch > 31) break;
		pNext_++;
	}
}


ParsedString Parser::string()
{
	ParsedString parsed;
	parsed.escapeCount = 0;
	char quoteChar = *pNext_;
	if (quoteChar != '\"' && quoteChar != '\'')
	{
		parsed.len = 0;
		parsed.str = nullptr;
		return parsed;
	}
	pNext_++;
	const char* pStart = pNext_;
	for (;;)
	{
		char ch = *pNext_;
		if (ch == quoteChar)
		{
			parsed.str = pStart;
			parsed.len = (pNext_ - pStart);
			pNext_++;
			return parsed;
		}
		if (ch == 0 || ch == '\n' || ch == '\r')
		{
			error("Unterminated string constant");
		}
		if (ch == '\\')
		{
			parsed.escapeCount++;
			pNext_++;
			if (*pNext_ == 0)
			{
				parsed.str = nullptr;
				parsed.len = pNext_ - pStart;
				return parsed;
			}
		}
		pNext_++;
	}
}


double Parser::number()
{
	char* pAfter;
	double value = strtod(pNext_, &pAfter);  
	if(pAfter == pNext_) return std::numeric_limits<double>::quiet_NaN();
	pNext_ = pAfter;
	skipWhitespace();
	return value;
}


std::string_view Parser::identifier(
	const CharSchema& validFirstChar,
	const CharSchema& validSubsequentChars)
{
	if (!validFirstChar.test(*pNext_)) return std::string_view();
	const char* pStart = pNext_;
	for (;;)
	{
		pNext_++;
		if (!validSubsequentChars.test(*pNext_))
		{
			size_t len = pNext_ - pStart;
			skipWhitespace();
			return std::string_view(pStart, len);
		}
	}
}

