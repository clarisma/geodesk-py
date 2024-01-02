// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include "TElement.h"
#include <common/util/pointer.h>

class TTile;

class TRelationTable : public TSharedElement
{
public:
	TRelationTable(int32_t loc, const uint8_t* data, uint8_t size) :
		TSharedElement(Type::RELTABLE, loc, data, size, Alignment::WORD)
	{
	}

	void write(const TTile& tile) const;
};