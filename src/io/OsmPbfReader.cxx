// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#include "OsmPbfReader.h"
#include <cassert>
#include <winsock2.h> // for ntohl()
// #include <arpa/inet.h> // for ntohl()
#include <boost/iostreams/device/mapped_file.hpp>
#include <zlib.h>
#include <common/util/log.h>
#include <common/util/varint.h>

template class TaskEngine<OsmPbfReaderContext, OsmPbfBlock>;

enum OsmPbf 
{
    BLOBHEADER_TYPE = (1 << 3) | 2,
    BLOBHEADER_DATASIZE = (3 << 3),

    BLOB_RAW_DATA = (1 << 3) | 2,
    BLOB_RAW_SIZE = (2 << 3),
    BLOB_ZLIB_DATA = (3 << 3) | 2,

    HEADER_BBOX = (1 << 3) | 2,
    HEADER_REQUIRED_FEATURES = (4 << 3) | 2,
    HEADER_OPTIONAL_FEATURES = (5 << 3) | 2,
    HEADER_WRITINGPROGRAM = (16 << 3) | 2,
    HEADER_SOURCE = (17 << 3) | 2,
    HEADER_REPLICATION_TIMESTAMP = (32 << 3),
    HEADER_REPLICATION_SEQUENCE = (33 << 3),
    HEADER_REPLICATION_URL = (34 << 3) | 2,

    BLOCK_STRINGTABLE = (1 << 3) | 2,
    BLOCK_GROUP = (2 << 3) | 2,
    BLOCK_GRANULARITY = 17 << 3,
    BLOCK_DATE_GRANULARITY = 18 << 3,
    BLOCK_LAT_OFFSET = 19 << 3,
    BLOCK_LON_OFFSET = 20 << 3,

    STRINGTABLE_ENTRY = (1 << 3) | 2,

    // Structures that appear within a PrimitiveGroup
    GROUP_NODE = (1 << 3) | 2,
    GROUP_DENSENODES = (2 << 3) | 2,
    GROUP_WAY = (3 << 3) | 2,
    GROUP_RELATION = (4 << 3) | 2,
    GROUP_CHANGESET = (5 << 3) | 2,

    DENSENODE_IDS = (1 << 3) | 2,
    DENSENODE_INFO = (5 << 3) | 2,
    DENSENODE_LATS = (8 << 3) | 2,
    DENSENODE_LONS = (9 << 3) | 2,
    DENSENODE_TAGS = (10 << 3) | 2,

    ELEMENT_ID = (1 << 3),
    ELEMENT_KEYS = (2 << 3) | 2,
    ELEMENT_VALUES = (3 << 3) | 2,
    ELEMENT_INFO = (4 << 3) | 2,

    WAY_NODES = (8 << 3) | 2,

    RELATION_MEMBER_ROLES = (8 << 3) | 2,
    RELATION_MEMBER_IDS = (9 << 3) | 2,
    RELATION_MEMBER_TYPES = (10 << 3) | 2
};


OsmPbfReader::OsmPbfReader(int maxThreads, int queueSize) :
	TaskEngine<OsmPbfReaderContext, OsmPbfBlock>(maxThreads, queueSize, true)
{

}

void OsmPbfReader::read(const char* fileName)
{
	boost::iostreams::mapped_file_source file;
	file.open(fileName);
	const uint8_t* p = (const uint8_t * )file.data();
	std::size_t fileSize = file.size();
    const uint8_t* pEnd = p + fileSize;

    while (p < pEnd)
    {
        uint32_t headerLen;
        memcpy(&headerLen, p, 4);
        headerLen = ntohl(headerLen);
        p += 4;
        const uint8_t* pHeaderEnd = p + headerLen;
        if (pHeaderEnd > pEnd)
        {
            // TODO: file truncated
        }

        std::string_view blockType;
        uint32_t dataLen = 0;
        while (p < pHeaderEnd)
        {
            uint32_t field = readVarint32(p);
            switch (field)
            {
            case BLOBHEADER_TYPE:
                blockType = readStringView(p);
                break;
            case BLOBHEADER_DATASIZE:
                dataLen = readVarint32(p);
                break;
            }
            // TODO: optional bytes indexdata = 2
        }
        if (blockType.empty() || dataLen == 0)
        {
            // TODO: invalid block header
        }

        assert(p == pHeaderEnd);

        if (blockType == "OSMData")
        {
            // LOG("OSMData (%d bytes)", dataLen);
            submit(OsmPbfBlock(pHeaderEnd, dataLen, dataLen + headerLen + 4));
        }
        else if (blockType == "OSMHeader")
        {
            // TODO: process header
            LOG("OSMHeader");
        }
        else
        {
            // TODO: unknown block type
        }
        p += dataLen;
    }
    shutdown();
    awaitCompletion();
}

void OsmPbfReader::process(OsmPbfReaderContext& context, OsmPbfBlock block)
{
    const uint8_t* p = block.pData_;
    const uint8_t* pEnd = p + block.dataLength_;
    const uint8_t* pRaw = nullptr;
    uint32_t rawLen = 0;
    const uint8_t* pZipped = nullptr;
    uint32_t zippedLen = 0;
    uint32_t uncompressedLen = 0;
    while (p < pEnd)
    {
        int32_t field = readVarint32(p);
        switch (field)
        {
        case BLOB_RAW_DATA:
            rawLen = readVarint32(p);
            pRaw = p;
            p += rawLen;
            break;
        case BLOB_RAW_SIZE:
            uncompressedLen = readVarint32(p); 
            break;
        case BLOB_ZLIB_DATA:
            zippedLen = readVarint32(p);
            pZipped = p;
            p += zippedLen;
            break;
        default:
            // TODO: throw new PbfException("Illegal tag: " + field);
            break;
        }
    }

    if (pZipped)
    {
        // LOG("Unzipping % d bytes into % d", zippedLen, uncompressedLen);
        uint8_t* pUnzipped = new uint8_t[uncompressedLen];
        if (inflateBlock(pZipped, zippedLen, pUnzipped, uncompressedLen) != Z_OK)
        {
            delete pUnzipped;
            // TODO: error
            return;
        }
        decodeBlock(pUnzipped, uncompressedLen);
        delete pUnzipped;
    }
    else if (pRaw)
    {
        decodeBlock(pRaw, uncompressedLen);
    }
}

int OsmPbfReader::inflateBlock(const uint8_t* pZipped, uint32_t zippedLen, 
    uint8_t* pUnzipped, uint32_t unzippedLen)
{
    z_stream strm;
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    strm.avail_in = zippedLen;
    strm.next_in = reinterpret_cast<Bytef*>(const_cast<uint8_t*>(pZipped));
    strm.avail_out = unzippedLen;
    strm.next_out = reinterpret_cast<Bytef*>(pUnzipped);

    int res = inflateInit(&strm);
    if (res != Z_OK) return res;
    res = inflate(&strm, Z_FINISH);
    if (res != Z_STREAM_END) 
    {
        inflateEnd(&strm);
        return res;
    }
    return inflateEnd(&strm);
}


void OsmPbfReader::decodeBlock(const uint8_t* pData, uint32_t len)
{

}

