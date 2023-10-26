// Copyright (c) 2023 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include <cassert>
#include <cstdint>
#include <string>


class ParseException : public std::exception 
{
public:
	ParseException(const std::string& msg) : message_(msg) {}

	virtual const char* what() const noexcept override 
	{
		return message_.c_str();
	}

private:
	std::string message_;
};

/**
 * A bitmap consisting of 256 flags that can be used to quickly 
 * identify whether a given character is part of a set (e.g. 
 * "valid first character of an identifier").
 * 
 * TODO: 128 would be enough for Latin1; non-Latin1 Unicode chars
 * need a different check anyway. (But may be cheaper to use same
 * lookup, cost is only 16 bytes)
 */
class CharSchema
{
public:
	CharSchema(uint64_t x0, uint64_t x1, uint64_t x2, uint64_t x3)
	{
		bits_[0] = x0;
		bits_[1] = x1;
		bits_[2] = x2;
		bits_[3] = x3;
	}

	bool test(char ch) const
	{
		uint8_t n = static_cast<uint8_t>(ch);
		assert(n >= 0 && n < 256);
		int slot = n / 64;
		uint64_t bit = 1ULL << (n % 64);
		return bits_[slot] & bit;
	}

private:
	uint64_t bits_[4];
};


struct ParsedString
{
	const char* str;
	uint32_t len;
	uint32_t escapeCount;

	bool isNull() const { return str == nullptr;  }
	std::string_view asStringView() const { return std::string_view(str, len); }
};

class Parser
{
public:
	Parser(const char* pInput) :
		pNext_(pInput), pStart_(pInput)
	{
	}

protected:
	/**
	 * Checks whether the next token is a string constant (in single or 
	 * double quotes). If successful, returns a ParsedString structure.
	 */
	ParsedString string();
	std::string_view identifier(const CharSchema& validFirstChar,
		const CharSchema& validSubsequentChars);
	double number();
	
	bool end() 
	{
		return *pNext_ == 0;
	}

	static const int32_t ESCAPED_STRING_FLAG = 0x8000'0000;
	static const int32_t INVALID_STRING = 0xffff'ffff;

	void skipWhitespace();

	void error(const char* format, ...);
		
	bool accept(char ch)
	{
		if (*pNext_ != ch) return false;
		pNext_++;
		skipWhitespace();
        return true;
	}

	void expect(char ch);
	
	const char* pNext_;		// slighly more efficient placing pNext first
	const char* pStart_;	
};
