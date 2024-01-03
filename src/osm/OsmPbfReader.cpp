// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#include "OsmPbfReader.h"
#include <fstream>
#include <iostream>
#include <string_view>
#if defined(_WIN32) 
#include <winsock2.h>  // for ntohl()
#else
#include <arpa/inet.h> // for ntohl()
#endif
#include <common/util/log.h>
#include <common/util/varint.h>


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


void OsmPbfReader::read(const char* fileName)
{
	std::ifstream file(fileName, std::ios::binary);
    file.seekg(0, std::ifstream::end);  // Move to the end of the file
    std::streamsize fileSize = file.tellg(); // Get the position (file size)
    file.seekg(0, std::ifstream::beg);   // Reset the position to the beginning
    
    size_t totalBytesRead = 0;
    while (totalBytesRead < fileSize)
    {
        uint32_t rawHeaderLen;
        file.read(reinterpret_cast<char*>(&rawHeaderLen), 4);
        uint32_t headerLen = ntohl(rawHeaderLen);

        if (headerLen > 256)
        {
            // TODO: excessive headr length
        }

        uint8_t buf[256];
        file.read(reinterpret_cast<char*>(buf), headerLen);
        const uint8_t* p = buf;
        const uint8_t* pEnd = p + headerLen;

        std::string_view blockType;
        uint32_t dataLen = 0;
        while (p < pEnd)
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
        }

        if (blockType.empty() || dataLen == 0)
        {
            // TODO: invalid block header
        }

        if (blockType == "OSMData")
        {
            // LOG("OSMData (%d bytes)", dataLen);
            uint8_t* data = new uint8_t[dataLen];
            file.read(reinterpret_cast<char*>(data), dataLen);
            delete[] data;  // TOOD
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
        totalBytesRead += dataLen + headerLen + 4;
    }
}


