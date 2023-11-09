#pragma once

#include "TElement.h"
#include <common/util/pointer.h>

class TTagTable : public TSharedElement
{
public:
	TTagTable(int32_t loc, const uint8_t* data, uint8_t size, uint8_t anchor) :
		TSharedElement(loc, data, size, Alignment::WORD),
		anchor_(anchor)
		// TODO: category
	{
	}

private:
	uint32_t anchor_;
	int32_t category_;
};