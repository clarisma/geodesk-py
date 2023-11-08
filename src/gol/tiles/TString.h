#pragma once

#include "TElement.h"

class TString : public TSharedElement
{
public:
	TString(int32_t loc, const uint8_t* data) :
		TSharedElement(loc, data, unaligned(readLength(data)))
	{
	}

	static uint32_t readLength(const uint8_t* data)
	{
		uint32_t len = *data;
		return (len & 0x80) ? (((len & 0xf7) | (*(data + 1) << 7)) + 2) : (len + 1);
	}
};
