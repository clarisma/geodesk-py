// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#include "Inflater.h"
#include <cassert>
#include <zlib.h>
#include "ZipException.h"

Inflater::Inflater()
{

}

size_t Inflater::inflate(File& file, uint64_t ofs, size_t compressedSize, 
    uint8_t* unzipped, size_t unzippedSize)
{
	if (!buffer_) buffer_.reset(new uint8_t[bufferSize_]);

	z_stream stream;
    stream.zalloc = Z_NULL;
    stream.zfree = Z_NULL;
    stream.opaque = Z_NULL;
    stream.avail_in = 0;
    stream.next_in = Z_NULL;
    stream.avail_out = unzippedSize;
    stream.next_out = unzipped;
    int ret = inflateInit(&stream);
    if (ret != Z_OK) return 0;

    size_t inputRemaining = compressedSize;

    while(inputRemaining)
    {
        try
        {
            stream.avail_in = file.read(ofs, buffer_.get(), 
                std::min(inputRemaining, bufferSize_));
        }
        catch (IOException& ex)
        {
            inflateEnd(&stream);
            throw;
        }
        inputRemaining -= stream.avail_in;
        stream.next_in = buffer_.get();

        ret = ::inflate(&stream, Z_NO_FLUSH);
        if (ret != Z_OK) break;
    }
    if (ret != Z_STREAM_END) throw ZipException(ret);
    inflateEnd(&stream);
    return stream.total_out;
}
