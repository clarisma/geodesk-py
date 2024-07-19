// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include "TElement.h"
#include "TIndex.h"
#include <common/util/pointer.h>

class IndexSettings;
class TTile;

class TTagTable : public TSharedElement
{
public:
	TTagTable(Handle handle, const uint8_t* data, uint32_t size, uint32_t anchor) :
		TSharedElement(Type::TAGS, handle, data, size, Alignment::WORD, anchor)
	{
		setCategory(TIndex::UNASSIGNED_CATEGORY);
	}

	bool hasLocalTags() const { return anchor() != 0; }
	void addStrings(Layout& layout) const;
	void write(const TTile& tile) const;
	uint32_t assignIndexCategory(const IndexSettings& indexSettings);

	TTagTable* nextTags() const
	{
		assert(next_ == nullptr || next_->type() == Type::TAGS);
		return reinterpret_cast<TTagTable*>(next_);
	}
};