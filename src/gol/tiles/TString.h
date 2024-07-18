// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include "TElement.h"

class TString : public TSharedElement
{
public:
	TString(int32_t loc, const uint8_t* data) :
		TSharedElement(Type::STRING, loc, data, getStringSize(data), Alignment::BYTE)
	{
	}

	bool operator<(const TSharedElement& other) const override
	{
		uint32_t ofs1 = 1 + (*data() >> 7);
		uint32_t ofs2 = 1 + (*other.data() >> 7);
		uint32_t len1 = size() - ofs1;
		uint32_t len2 = other.size() - ofs2;
		uint32_t commonLen = std::min(len1, len2);
		int res = memcmp(data() + ofs1, other.data() + ofs2, commonLen);
		if (res == 0)
		{
			return len1 < len2;
		}
		return res < 0;
	}

	static uint32_t getStringSize(const uint8_t* data)
	{
		uint32_t len = *data;
		return (len & 0x80) ? (((len & 0xf7) | (*(data + 1) << 7)) + 2) : (len + 1);
	}
};
