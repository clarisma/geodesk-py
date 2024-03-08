// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once
#include <memory>
#include <common/io/File.h>

class Inflater
{
public:
	Inflater();
	size_t inflate(File& file, uint64_t ofs, size_t compressedSize, 
		uint8_t* unzipped, size_t unzippedSize);

private:
	std::unique_ptr<uint8_t[]> buffer_;
	size_t bufferSize_ = 64 * 1024;
};
