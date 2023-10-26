// Copyright (c) 2023 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once
#include <cstdint>
#include <regex>
#include <string_view>
#include "Matcher.h"


class MatcherEngine
{
public:
	static int accept(const Matcher*, const uint8_t*);

private:
	void jumpIf(int matched) { ip_ += matched ? ip_.getShort() : 2; }
	inline int scanGlobalKeys();
	int scanLocalKeys();	// inline not needed for this
	static inline int isNegated(int op) { return (op >> 8) & 1; }
		// TODO: Is negate the only flag? If so, no need for AND
	inline std::string_view getStringOperand();
	static inline std::string_view asStringView(const uint8_t* stringValue)
	{
		LocalString val(stringValue);
		return val.toStringView();
	}
	inline double getDoubleOperand();
	inline const std::regex* getRegexOperand();
	inline uint32_t getFeatureTypeOperand();

	pointer ip_;
	const uint8_t* pTagTable_;	// with local-keys flag as bit 0
	pointer pTag_;
	uint16_t tagKey_;
	int16_t valueOfs_;
};

