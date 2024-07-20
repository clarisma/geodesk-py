// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include "TElement.h"
#include "TIndex.h"
#include <common/util/DataPtr.h>

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
	TTagTable(Handle handle, const uint8_t* data, uint32_t size, uint32_t anchor) :
		TSharedElement(TYPE, handle, data, size, Alignment::WORD, anchor)
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

	class Hasher
	{
	public:
		Hasher() noexcept : hash_(5381) {};		// djb2 start value
		void tag(uint16_t key, uint32_t value) noexcept
		{
			add(key);
			add(value);
		}
		void tag(uint16_t key, TString* value) noexcept
		{
			add(key);
			hash_ ^= value->hash();
		}
		void tag(TString* key, uint32_t value) noexcept
		{
			hash_ ^= key->hash();
			add(value);
		}
		void tag(TString* key, TString* value) noexcept
		{
			hash_ ^= key->hash();
			hash_ ^= value->hash();
		}

		size_t hash() const noexcept { return hash_; }

	private:
		void add(uint32_t v) noexcept
		{
			hash_ = ((hash_ << 5) + hash_) + v;
		}

		size_t hash_;
	};

	static constexpr TElement::Type TYPE = TElement::Type::TAGS;
};