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
		TSharedElement(Type::TAGS, loc, data, size, Alignment::WORD, anchor),
		category_(TIndex::UNASSIGNED_CATEGORY),
		users_(0)
	{
	}

	bool hasLocalTags() const { return anchor() != 0; }
	int category() const { return category_; }
	void setCategory(int category) { category_ = category; }
	void addStrings(Layout& layout) const;
	void write(const TTile& tile) const;
	uint32_t assignIndexCategory(const IndexSettings& indexSettings);

	TTagTable* nextTags() const
	{
		assert(next_ == nullptr || next_->type() == Type::TAGS);
		return reinterpret_cast<TTagTable*>(next_);
	}

private:
	unsigned int category_ :  8;
	// TODO: extra flags?
	uint32_t users_;
};