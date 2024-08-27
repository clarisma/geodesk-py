// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include <common/alloc/Block.h>
#include "ZipException.h"

namespace Zip
{
ByteBlock deflate(const uint8_t* data, size_t size);
inline ByteBlock deflate(const ByteBlock& block)
{
    return deflate(block.data(), block.size());
}

ByteBlock inflate(const uint8_t* data, size_t size, size_t sizeUncompressed);
inline ByteBlock inflate(const ByteBlock& block, size_t sizeUncompressed)
{
    return inflate(block.data(), block.size(), sizeUncompressed);
}

uint32_t calculateChecksum(const ByteBlock& block);
void verifyChecksum(const ByteBlock& block, uint32_t checksum);
}
    
