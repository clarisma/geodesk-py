// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include "Buffer.h"
#include "varint.h"
#include <cassert>
#include <cstdint>
#include <cstring>
#include <string>
#ifdef GEODESK_PYTHON
#include <Python.h>
#endif
#include <common/data/Span.h>

// TODO: use p_ and end_ of buffer, enable buffer to be safely used by multiple
// writers, and avoid need to flush for memory-based buffers

// TODO: should destructor auto-flush?

class BufferWriter
{
public:
	BufferWriter(Buffer* buf) :
		buf_(buf)
	{
		p_ = buf->pos();
		end_ = buf->end();
	}

	BufferWriter() {}  // TODO: this is dangerous, fix this
		// Needed for StringBuilder because of init order
		// Maybe make private and make StringBuilder friend?
	void setBuffer(Buffer* buf) 
	{
		buf_ = buf;
		p_ = buf->pos();
		end_ = buf->end();
	}

	// TODO: make const uint8_t instead?
	const char* data() const
	{
		return buf_->data();
	}

	ByteSpan span() const
	{
		return ByteSpan(reinterpret_cast<const uint8_t*>(buf_->data()), length());
	}

	// OK to implcitly convert to Bytespan 
	operator ByteSpan() const
	{
		return span();
	}

	const size_t length() const
	{
		return p_ - buf_->start();
	}

	const size_t size() const		// same as length(); TODO: standardize
	{
		return p_ - buf_->start();
	}

	bool isEmpty() const
	{
		return p_ == buf_->start();
	}

	void clear()
	{
		p_ = buf_->start();
	}


	void flush()
	{
		buf_->flush(p_);
		p_ = buf_->pos();
		end_ = buf_->end();
	}

	size_t capacityRemaining() const
	{
		return end_ - p_;
	}

	/*
	size_t length() const
	{
		return p_ - buf_;
	}
	*/

	/*	// can't really ensure capacity
	void ensureCapacity(size_t len)
	{
		if (capacityRemaining() < len) flush();
	}
	*/

	void writeByte(char b)
	{
		*p_++ = b;
		if (p_ == end_) filled();
	}

	void writeBytes(const void* data, size_t len)
	{
		const char* b = reinterpret_cast<const char*>(data);
		for (;;)
		{
			size_t remainingCapacity = capacityRemaining();
			if (len < remainingCapacity)
			{
				std::memcpy(p_, b, len);
				p_ += len;
				return;
			}
			std::memcpy(p_, b, remainingCapacity);
			p_ += remainingCapacity;
			filled();
			b += remainingCapacity;
			len -= remainingCapacity;
		}
	}

	/*
	void writeBytes(const uint8_t* b, size_t len)
	{
		writeBytes(reinterpret_cast<const char*>(b), len);
	}

	void writeBytes(const void* b, size_t len)
	{
		writeBytes(reinterpret_cast<const char*>(b), len);
	}
	*/

	template<typename T>
	void writeBinary(const T& obj)
	{
		writeBytes(reinterpret_cast<const char*>(&obj), sizeof(T));
	}

	void writeVarint(uint64_t v)
	{
		uint8_t buf[10];
		uint8_t* p = buf;
		::writeVarint(p, v);
		writeBytes(buf, p - buf);
	}

	void writeSignedVarint(int64_t v)
	{
		uint8_t buf[10];
		uint8_t* p = buf;
		::writeSignedVarint(p, v);
		writeBytes(buf, p - buf);
	}


	template <size_t N>
	void writeConstString(const char(&s)[N])
	{
		writeBytes(s, N-1); // Subtract 1 to exclude null terminator
	}

	void writeString(const char* s)
	{
		writeBytes(s, strlen(s));
	}

	void writeString(std::string_view sv)
	{
		writeBytes(sv.data(), sv.length());
	}

	#ifdef GEODESK_PYTHON
	void writeString(PyObject* str)
	{
		Py_ssize_t len;
		const char* s = PyUnicode_AsUTF8AndSize(str, &len);
		if (!s)
		{
			PyErr_Clear();
			return;
		}
		writeBytes(s, len);
	}
	#endif

	void writeReplacedString(const char* s, const char* find, size_t findLen, 
		const char* replaceWith, size_t replaceLen);

	template <size_t NF>
	void writeReplacedString(const char* s, const char(&find)[NF], const char* replaceWith)
	{
		size_t replaceLen = strlen(replaceWith);
		writeReplacedString(s, find, NF-1, replaceWith, replaceLen);
	}

	template <size_t NF>
	void writeReplacedString(const char* s, const char(&find)[NF], 
		const char* replaceWith, size_t replaceLen)
	{
		writeReplacedString(s, find, NF-1, replaceWith, replaceLen);
	}

	template <size_t NF, size_t NR>
	void writeReplacedString(const char* s, const char(&find)[NF], const char(&replaceWith)[NR])
	{
		writeReplacedString(s, find, NF-1, replaceWith, NR-1);
	}

	void formatInt(int64_t d);	// TODO: rename
	void formatUnsignedInt(uint64_t v);
	void formatDouble(double d, int precision = 15, bool zeroFill = false);

	void writeJsonEscapedString(const char* s, size_t len);
	void writeJsonEscapedString(std::string_view sv)
	{
		writeJsonEscapedString(sv.data(), sv.length());
	}

	void writeRepeatedChar(char ch, int times)
	{
		// TODO: use smarter approach?
		for (int i = 0; i < times; i++) writeByte(ch);
	}

	BufferWriter& operator<<(char ch)
	{
		writeByte(ch);
		return *this;
	}

	BufferWriter& operator<<(uint32_t value)
	{
		formatUnsignedInt(value);
		return *this;
	}

	BufferWriter& operator<<(uint64_t value)
	{
		formatUnsignedInt(value);
		return *this;
	}

	//TODO: remove
	template<typename T>
	BufferWriter& operator<<(const T& value) 
	{
		value.write(*this);
		return *this;
	}

protected:
	static inline char* formatUnsignedLongReverse(unsigned long long d, char* end)
	{
		do
		{
			lldiv_t result = lldiv(d, 10);
			*(--end) = static_cast<char>('0' + result.rem);
			d = result.quot;
		} 
		while (d != 0);
		return end;
	}

	static inline char* formatFractionalReverse(unsigned long long d, char** pEnd, int precision, bool zeroFill)
	{
		char* end = *pEnd;
		char* start = end - precision;
		char* p = end;
		while(p > start)
		{
			lldiv_t result = lldiv(d, 10);
			if (p == end && result.rem == 0 && !zeroFill)
			{
				end--;		// skip trailing zeroes
				p--;
			}
			else
			{
				*(--p) = static_cast<char>('0' + result.rem);
			}
			d = result.quot;
		} 
		*pEnd = end;
		return p;
	}

	static inline char* formatLongReverse(long long d, char* end, bool negative)
	{
		d = (d < 0) ? -d : d;
		end = formatUnsignedLongReverse(d, end);
		*(end - 1) = '-';
		return end - negative;
	}

private:
	void filled()
	{
		buf_->filled(p_);
		p_ = buf_->pos();
		end_ = buf_->end();
	}

	Buffer* buf_;
	char* p_;
	char* end_;
};
