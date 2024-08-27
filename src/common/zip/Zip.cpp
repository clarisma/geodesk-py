// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#include "Zip.h"
#include <zlib.h>

namespace Zip
{
ByteBlock deflate(const uint8_t* data, size_t size)
{
    std::unique_ptr<uint8_t[]> compressed = 
        std::make_unique<uint8_t[]>(compressBound(size));
    uLongf compressedSize;
    int res = compress(compressed.get(), &compressedSize, data, size);
    if (res != Z_OK) throw ZipException(res);
    return ByteBlock(std::move(compressed), compressedSize);
}

ByteBlock inflate(const uint8_t* data, size_t size, size_t sizeUncompressed)
{
    // Allocate memory for the uncompressed data
    std::unique_ptr<uint8_t[]> uncompressedData(new uint8_t[sizeUncompressed]);

    /*
    // Initialize the zlib stream
    z_stream stream;
    stream.avail_in = static_cast<uInt>(size);
    stream.next_in = const_cast<uint8_t*>(data);
    stream.avail_out = static_cast<uInt>(sizeUncompressed);
    stream.next_out = uncompressedData.get();

    // Initialize the inflation process
    int ret = inflateInit(&stream);
    if (ret != Z_OK)
    {
        throw ZipException(ret);
    }

    // Inflate the data
    ret = inflate(&stream, Z_FINISH);
    if (ret != Z_STREAM_END)
    {
        inflateEnd(&stream);
        throw ZipException(ret);
    }

    // Clean up
    inflateEnd(&stream);
    

    // Verify the uncompressed size
    if (stream.total_out != sizeUncompressed)
    {
        throw ZipException("Uncompressed size does not match expected size");
    }
    */

    uLongf uncompressedSizeZlib = sizeUncompressed;
    int res = uncompress(uncompressedData.get(), &uncompressedSizeZlib, data, size);
    if (res != Z_OK)
    {
        throw ZipException(res);
    }
    return ByteBlock(std::move(uncompressedData), sizeUncompressed);
}


uint32_t calculateChecksum(const ByteBlock& block)
{
    return crc32_z(0, block.data(), static_cast<z_size_t>(block.size()));
}

void verifyChecksum(const ByteBlock& block, uint32_t checksum)
{
    if (calculateChecksum(block) != checksum)
    {
        throw ZipException("Checksum mismatch");
    }
}

}  // end namespace Zip

