// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once
#include <cassert>
#include <cstdio>
#include <common/alloc/Block.h>

class Buffer
{
public:
	Buffer() : buf_(nullptr) {}
	virtual ~Buffer() {}

	const char* data() const { return buf_; }
	char* pos() const { return p_; }
	char* start() const { return buf_; }
	char* end() const { return end_; }
	size_t length() const { return p_ - buf_; }
	size_t capacity() const { return end_ - buf_; }
	bool isEmpty() const { return p_ == buf_; }

	virtual void filled(char *p) = 0;
	virtual void flush(char* p) = 0;

protected:
	char* buf_;
	char* p_;
	char* end_;
};


class DynamicBuffer : public Buffer
{
public:
	DynamicBuffer(size_t initialCapacity);
	virtual ~DynamicBuffer();

	DynamicBuffer(DynamicBuffer&& other) noexcept
	{
		buf_ = other.buf_;
		p_ = other.p_;
		end_ = other.end_;
		other.buf_ = nullptr;
		other.p_ = nullptr;
		other.end_ = nullptr;
	}

	DynamicBuffer& operator=(DynamicBuffer&& other) noexcept
	{
		if (this != &other)
		{
			if(buf_) delete[] buf_;
			buf_ = other.buf_;
			p_ = other.p_;
			end_ = other.end_;
			other.buf_ = nullptr;
			other.p_ = nullptr;
			other.end_ = nullptr;
		}
		return *this;
	}

	// Disable copy constructor and copy assignment operator
	DynamicBuffer(const DynamicBuffer&) = delete;
	DynamicBuffer& operator=(const DynamicBuffer&) = delete;

	virtual void filled(char* p);		// TODO: why does this take a pointer??
	virtual void flush(char* p);
	ByteBlock takeBytes();

protected:
	void grow();
};

/*
class FrugalBuffer : public DynamicBuffer
{
public:
	FrugalBuffer(size_t initialHeapCapacity);
	virtual ~DynamicBuffer() {};

	virtual void flush();

protected:
	void grow();
};
*/

class FileBuffer : public Buffer
{
public:
	FileBuffer(FILE* file, size_t capacity);
	virtual ~FileBuffer();
	
	virtual void filled(char* p);
	virtual void flush(char* p);

private:
	FILE* file_;
};
