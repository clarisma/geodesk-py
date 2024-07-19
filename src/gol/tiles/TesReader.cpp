// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#include "TesReader.h"
#include <common/util/varint.h>

TesReader::TesReader()
{

}


void TesReader::readString()
{
	uint32_t size = TString::getStringSize(p_);

	// Check if string exists already

	uint8_t* copy = tile_.alloc(size);
	memcpy(copy, p_, size);
	tile_.addString()
	p_ += size;
}
