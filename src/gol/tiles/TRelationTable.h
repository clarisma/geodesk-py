// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include "TElement.h"
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

	static constexpr TElement::Type TYPE = TElement::Type::RELTABLE;
};