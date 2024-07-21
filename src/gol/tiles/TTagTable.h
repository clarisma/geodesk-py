// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include "TElement.h"
#include "TIndex.h"
#include <common/util/DataPtr.h>
#include <feature/types.h>

class IndexSettings;
class TTile;

// TODO: We cannot simply compare the binary data of two tab-tables
// Local-key pointers may have the same bit pattern as a global-key encoding
// leading to an extremely rare but possible bug
// We need to store the number of local tags as part of the binary data
// alternatively, compare the anchor in addition to size and content

class TTagTable : public TSharedElement
{
public:
	TTagTable(Handle handle, const uint8_t* data, uint32_t size, 
		uint32_t hash, uint32_t anchor) :
		TSharedElement(TYPE, handle, data, size, Alignment::WORD, hash, anchor)
	{
		setCategory(TIndex::UNASSIGNED_CATEGORY);
	}

	bool hasLocalTags() const { return anchor() != 0; }
	void addStrings(Layout& layout) const;
	void write(const TileKit& tile) const;
	uint32_t assignIndexCategory(const IndexSettings& indexSettings);

	TTagTable* nextTags() const
	{
		assert(next_ == nullptr || next_->type() == Type::TAGS);
		return reinterpret_cast<TTagTable*>(next_);
	}

	// TODO: does key include last-item marker?
	// Keys must be stripped of all flags, just the code
	class Hasher
	{
	public:
		Hasher() noexcept : hash_(5381) {};		// djb2 start value
		
		size_t hash() const noexcept { return hash_; }

		void addKey(uint32_t k) noexcept
		{
			assert(k <= FeatureConstants::MAX_COMMON_KEY);
			addValue(k);
		}

		void addKey(TString* v) noexcept
		{
			addValue(v);
		}

		void addValue(uint32_t v) noexcept
		{
			hash_ = ((hash_ << 5) + hash_) + v;
		}

		void addValue(TString* v) noexcept
		{
			hash_ ^= v->hash();
		}

		size_t hash_;
	};

	static constexpr TElement::Type TYPE = TElement::Type::TAGS;
};