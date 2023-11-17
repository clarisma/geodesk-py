#pragma once

#include "TElement.h"
#include "TIndex.h"
#include <common/util/pointer.h>

class IndexSettings;
class TTile;

class TTagTable : public TSharedElement
{
public:
	TTagTable(int32_t loc, const uint8_t* data, uint32_t size, uint32_t anchor) :
		TSharedElement(loc, data, size, Alignment::WORD),
		anchor_(anchor),
		category_(TIndex::UNASSIGNED_CATEGORY),
		users_(0)
	{
	}

	bool hasLocalTags() const { return anchor_ != 0; }
	int category() const { return category_; }
	void setCategory(int category) { category_ = category; }
	void write(const TTile* tile, uint8_t* p) const;
	uint32_t assignIndexCategory(const IndexSettings& indexSettings);

private:
	unsigned int anchor_ :   24;
	unsigned int category_ :  8;
	uint32_t users_;
};