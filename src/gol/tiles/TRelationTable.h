#pragma once

#include "TElement.h"
#include <common/util/pointer.h>

class TRelationTable : public TSharedElement
{
public:
	TRelationTable(int32_t loc, const uint8_t* data, uint8_t size) :
		TSharedElement(loc, data, size, Alignment::WORD)
	{
	}
};