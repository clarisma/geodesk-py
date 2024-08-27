// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once
#include <common/io/File.h>
#include <common/util/Buffer.h>

class FileBuffer2 : public Buffer
{
public:
	FileBuffer2(size_t capacity = 64 * 1024)
	{
		buf_ = new char[capacity];
		p_ = buf_;
		end_ = buf_ + capacity;
	}

	virtual ~FileBuffer2()
	{
		if (p_ > buf_) file_.write(buf_, p_ - buf_);
		if (buf_) delete[] buf_;
	}

	void open(const char* filename, int /* OpenMode */ mode =
		File::OpenMode::CREATE | File::OpenMode::WRITE | File::OpenMode::REPLACE_EXISTING)
	{
		file_.open(filename, mode);
	}

	virtual void filled(char* p)
	{
		flush(p);
	}

	virtual void flush(char* p)
	{
		file_.write(buf_, p - buf_);
		p_ = buf_;
	}

private:
	File file_;
};

