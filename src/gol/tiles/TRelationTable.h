// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include "TElement.h"
#include <common/util/pointer.h>

class TTile;

class TRelationTable : public TSharedElement
{
public:
	TRelationTable(Handle handle, const uint8_t* data, uint8_t size) :
		TSharedElement(Type::RELTABLE, handle, data, size, Alignment::WORD)
	{
	}

	void write(const TTile& tile) const;
};