// Copyright (c) 2023 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once
#include <cassert>
#include <cstdio>

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

	virtual void filled(char* p);
	virtual void flush(char* p);
	char* take();

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
