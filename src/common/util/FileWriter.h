// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once
#include "BufferWriter.h"
#include "FileBuffer2.h"

class FileWriter : public BufferWriter
{
public:
	FileWriter() : BufferWriter(&buf_)
	{
	}

	FileWriter(const char* filename, size_t capacity = 64 * 1024) : 
		BufferWriter(&buf_),
		buf_(capacity)
	{
		open(filename);
	}

	void open(const char* filename)
	{
		buf_.open(filename);
	}

private:
	FileBuffer2 buf_;
};
