// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once
#include "BufferWriter.h"
#include <filesystem>
#include "FileBuffer2.h"

class FileWriter : public BufferWriter
{
public:
	FileWriter()
	{
		setBuffer(&buf_);
	}

	FileWriter(const char* filename, size_t capacity = 64 * 1024) : 
		buf_(capacity)
	{
		setBuffer(&buf_);
		open(filename);
	}

	FileWriter(std::filesystem::path path, size_t capacity = 64 * 1024) :
		buf_(capacity)
	{
		setBuffer(&buf_);
		open(path);
	}

	~FileWriter()
	{
		flush();		// TODO: should base class always flush?
	}

	void open(const char* filename)
	{
		setBuffer(&buf_);
		buf_.open(filename);
	}

	void open(std::filesystem::path path)
	{
		buf_.open(path.string().c_str());
	}

private:
	FileBuffer2 buf_;
};
