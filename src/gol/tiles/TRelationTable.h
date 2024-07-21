// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include "TElement.h"
#include "TFeature.h"
#include <common/util/pointer.h>

class TileKit;

class TRelationTable : public TSharedElement
{
public:
	TRelationTable(Handle handle, const uint8_t* data, uint8_t size, uint32_t hash) :
		TSharedElement(TYPE, handle, data, size, Alignment::WORD, hash)
	{
	}

	void write(const TileKit& tile) const;

	class Hasher
	{
	public:
		Hasher() noexcept : hash_(5381) {};		// djb2 start value

		size_t hash() const noexcept { return hash_; }

		void addLocalRelation(TRelation* rel) noexcept
		{
			addValue(rel->id());
		}

		void addForeignRelation(int tip, int tex)
		{
			addValue(tip);
			addValue(tex);
		}

	private:
		void addValue(uint64_t v) noexcept
		{
			hash_ = ((hash_ << 5) + hash_) + v;
		}

		size_t hash_;
	};

	static constexpr TElement::Type TYPE = TElement::Type::RELTABLE;
};