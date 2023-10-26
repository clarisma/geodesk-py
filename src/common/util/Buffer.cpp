// Copyright (c) 2023 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#include "Buffer.h"
#include <cstring>

FileBuffer::FileBuffer(FILE* file, size_t capacity) :
	file_(file)
{
	buf_ = new char[capacity];
	p_ = buf_;
	end_ = buf_ + capacity;
}


FileBuffer::~FileBuffer()
{
	if (buf_) delete[] buf_;
    fclose(file_);
}

void FileBuffer::filled(char* p)
{
    flush(p);
}

void FileBuffer::flush(char* p)
{
    assert(p >= buf_);
    assert(p <= end_);
	size_t written = fwrite(buf_, 1, p - buf_, file_);
	// TODO: check number of bytes written (error handling)
	p_ = buf_;
}


DynamicBuffer::DynamicBuffer(size_t initialCapacity)
{
	buf_ = new char[initialCapacity];
	p_ = buf_;
	end_ = buf_ + initialCapacity;
}


DynamicBuffer::~DynamicBuffer()
{
	if (buf_) delete[] buf_;
}


void DynamicBuffer::grow()
{
	size_t newCapacity = (end_ - buf_) * 2;
	size_t len = length();
	char* newBuf = new char[newCapacity];
	memcpy(newBuf, buf_, len);
	p_ += newBuf - buf_;
	buf_ = newBuf;
	end_ = newBuf + newCapacity;
}

void DynamicBuffer::filled(char* p)
{
    assert(p >= buf_);
    assert(p <= end_);
    p_ = p;
	char* oldBuf = buf_;
	grow();
	delete[] oldBuf;
}

void DynamicBuffer::flush(char* p)
{
    assert(p >= buf_);
    assert(p <= end_);
    p_ = p;
}


/*

#include <cstddef>
#include <cstring>
#include <memory>

template <std::size_t StackSize = 256>
class DynamicBuffer 
{
private:
    std::size_t size_ = 0;
    std::size_t capacity_ = StackSize;
    char stackData_[StackSize];
    std::unique_ptr<char[]> heapData_;

public:
    DynamicBuffer() = default;

    void append(const char* data, std::size_t len) 
    {
        ensureCapacity(size_ + len);
        if (heapData_) 
        {
            std::memcpy(heapData_.get() + size_, data, len);
        } 
        else 
        {
            std::memcpy(stackData_ + size_, data, len);
        }
        size_ += len;
    }

    const char* data() const 
    {
        return heapData_ ? heapData_.get() : stackData_;
    }

    std::size_t size() const 
    {
        return size_;
    }

private:
    void ensureCapacity(std::size_t required) 
    {
        if (required <= capacity_) return;

        while (capacity_ < required) 
        {
            capacity_ *= 2;
        }

        auto newBuffer = std::make_unique<char[]>(capacity_);
        const char* oldData = heapData_ ? heapData_.get() : stackData_;
        std::memcpy(newBuffer.get(), oldData, size_);
        heapData_ = std::move(newBuffer);
    }
};

int main() 
{
    DynamicBuffer<> buffer;
    const char* hello = "Hello, ";
    const char* world = "World!";
    buffer.append(hello, strlen(hello));
    buffer.append(world, strlen(world));

    // Use the buffer...
    // For example:
    printf("%s\n", buffer.data());
    return 0;
}

*/
