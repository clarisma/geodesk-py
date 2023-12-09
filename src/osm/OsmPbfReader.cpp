// Copyright (c) 2023 Clarisma / GeoDesk contributors
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
#include <zlib.h>
#include <common/util/log.h>
// #include <crtdbg.h>     // DEBUG on windows only!

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

template <typename Derived, typename WorkContext, typename OutputTask>
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
            throw OsmPbfException("Excessive header length (%d)", headerLen);
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
            throw OsmPbfException("Invalid blob header at offset %016llX", totalBytesRead);
        }

        OsmPbfBlock block;
        uint8_t* data = new uint8_t[dataLen];
        block.data = data;
        block.dataSize = dataLen;
        block.blockSize = dataLen + headerLen + 4;
        file.read(reinterpret_cast<char*>(data), dataLen);
        if (blockType == "OSMData")
        {
            postWork(block);
        }
        else if (blockType == "OSMHeader")
        {
            decodeHeaderBlock(block);
        }
        else
        {
            throw OsmPbfException("Unknown header type: %s", std::string(blockType).c_str());
        }
        totalBytesRead += block.blockSize;
    }

    LOG("Done.");
}


template <typename Derived, typename WorkContext, typename OutputTask>
void OsmPbfReader::decodeHeaderBlock(const OsmPbfBlock& block)
{
    UncompressedBlock uncompressed = uncompressBlock(block);
    // TODO
}

template <typename Derived, typename WorkContext, typename OutputTask>
UncompressedBlock OsmPbfReader::uncompressBlock(const OsmPbfBlock& block)
{
    const uint8_t* pRaw = nullptr;
    const uint8_t* pZipped = nullptr;
    uint32_t rawSize;
    uint32_t zippedSize;
    uint32_t uncompressedSize = 0;

    const uint8_t* p = block.data;
    const uint8_t* pEnd = p + block.dataSize;
    while (p < pEnd)
    {
        uint32_t field = readVarint32(p);
        switch (field)
        {
        case BLOB_RAW_DATA:
            rawSize = readVarint32(p);
            pRaw = p;
            p += rawSize;
            break;
        case BLOB_RAW_SIZE:
            uncompressedSize = readVarint32(p);
            break;
        case BLOB_ZLIB_DATA:
            zippedSize = readVarint32(p);
            pZipped = p;
            p += zippedSize;
            break;
        default:
            throw OsmPbfException("Unrecognized field: %d", field);
            break;
        }
    }

    UncompressedBlock uncompressed;
    if (pRaw)
    {
        uncompressed.data = pRaw;
        uncompressed.dataSize = rawSize;
        uncompressed.toDelete = block.data;
    }
    else if(pZipped)
    {
        if (uncompressedSize == 0)
        {
            throw OsmPbfException("Invalid uncompressed size: %d", uncompressedSize);
        }
        uint8_t* pUnzipped = new uint8_t[uncompressedSize];
        uLongf uncompressedSizeZlib = uncompressedSize;
        int result = uncompress(pUnzipped, &uncompressedSizeZlib, pZipped, zippedSize);
        delete block.data;
        if (result != Z_OK) 
        {
            throw OsmPbfException("Inflating failed with error code %d", result);
        }

        uncompressed.data = pUnzipped;
        uncompressed.dataSize = uncompressedSize;
        uncompressed.toDelete = pUnzipped;
    }
    else
    {
        throw OsmPbfException("Block has no content");
    }
    return std::move(uncompressed);
}


void OsmPbfContext::processTask(OsmPbfBlock& block)
{
    UncompressedBlock uncompressed = OsmPbfReader::uncompressBlock(block);
    const uint8_t* p = uncompressed.data;
    const uint8_t* pEnd = p + uncompressed.dataSize;

    granularity_ = 100;
    uint32_t dateGranularity = 1000;
    latOffset_ = 0;
    lonOffset_ = 0;

    while (p < pEnd)
    {
        uint32_t field = readVarint32(p);
        switch (field)
        {
        case BLOCK_STRINGTABLE:
            assert(strings_.empty());
            readStringTable(protobuf::readMessage(p));
            break;
        case BLOCK_GROUP:
            groups_.push_back(protobuf::readMessage(p));
            break;
        case BLOCK_GRANULARITY:
            granularity_ = readVarint32(p);
            break;
        case BLOCK_DATE_GRANULARITY:
            dateGranularity = readVarint32(p);
            break;
        case BLOCK_LAT_OFFSET:
            latOffset_ = readVarint64(p);        // TODO: signed??
            break;
        case BLOCK_LON_OFFSET:
            lonOffset_ = readVarint64(p);        // TODO: signed??
            break;
        default:
            throw OsmPbfException("Unrecognized field: %d", field);
            break;
        }
    }

    // Need to ensure that we read the granularity info
    // before we start decoding the primitive groups

    for (const auto& group : groups_) 
    {
        decodePrimitiveGroup(group);
    }

    strings_.clear();
    groups_.clear();
}


void OsmPbfContext::readStringTable(protobuf::Message strings)
{
    // assert(_CrtCheckMemory());
    const uint8_t* p = strings.p;
    while (p < strings.pEnd)
    {
        uint32_t marker = readVarint32(p);
        if (marker != STRINGTABLE_ENTRY)
        {
            throw OsmPbfException("Bad string table. Unexpected field:  %d", marker);
        }
        const uint8_t* pString = p;
        p += readVarint32(p);
        strings_.push_back(pString);
    }
    assert(p == strings.pEnd);
}

void OsmPbfContext::decodePrimitiveGroup(protobuf::Message group)
{
    const uint8_t* p = group.p;
    while (p < group.pEnd)
    {
        uint32_t field = readVarint32(p);
        switch (field)
        {
        case GROUP_NODE:
            throw OsmPbfException("Only dense nodes are supported");
            break;
        case GROUP_DENSENODES:
            // TODO: if (currentPhase != PHASE_NODES) switchPhase(PHASE_NODES);
            decodeDenseNodes(protobuf::readMessage(p));
            break;
        case GROUP_WAY:
            // TODO: if (currentPhase != PHASE_WAYS) switchPhase(PHASE_WAYS);
            decodeWay(protobuf::readMessage(p));
            break;
        case GROUP_RELATION:
            // TODO: if (currentPhase != PHASE_RELATIONS) switchPhase(PHASE_RELATIONS);
            decodeRelation(protobuf::readMessage(p));
            break;
        case GROUP_CHANGESET:
            protobuf::readMessage(p);
            break;
        default:
            protobuf::skipEntity(p, field);
            break;
        }
    }
    assert(p == group.pEnd);
}


void OsmPbfContext::decodeDenseNodes(protobuf::Message data)
{
    const uint8_t* p = data.p;
    protobuf::Message ids;
    protobuf::Message lats;
    protobuf::Message lons;
    protobuf::Message tags;
    
    while (p < data.pEnd)
    {
        protobuf::Field field = protobuf::readField(p);
        switch (field)
        {
        case DENSENODE_IDS:
            ids = protobuf::readMessage(p);
            break;
        case DENSENODE_INFO:
            protobuf::readMessage(p);
            break;
        case DENSENODE_LATS:
            lats = protobuf::readMessage(p);
            break;
        case DENSENODE_LONS:
            lons = protobuf::readMessage(p);
            break;
        case DENSENODE_TAGS:
            tags = protobuf::readMessage(p);
            break;
        default:
            protobuf::skipEntity(p, field);
            break;
        }
    }
    assert(p == data.pEnd);
    if (ids.p)
    {
        // TODO: check other messages are present
              

        // TODO: DenseTags tags = new DenseTags(strings, tagsBuf);

        /*
        Tags tags = tagsBuf==null ? EMPTY_TAGS :
            new DenseTags(strings, tagsBuf);
         */
         // TODO: guard against empty buffer (if none of nodes have tags)
         // TODO: fails if client does not read all tags, or keeps
         //  calling next() after last item --> make DenseTags more robust

        int64_t id = 0;
        int64_t lat = 0;
        int64_t lon = 0;
        while(ids.p < ids.pEnd)
        {
            id += readSignedVarint64(ids.p);
            lat += readSignedVarint64(lats.p);
            lon += readSignedVarint64(lons.p);
            int64_t latInNanoDeg = (latOffset_ + (granularity_ * lat));
            int64_t lonInNanoDeg = (lonOffset_ + (granularity_ * lon));

            this->node(id, static_cast<int32_t>(lonInNanoDeg / 100),
                static_cast<int32_t>(latInNanoDeg / 100), tags);
        }
        assert(lats.isEmpty());
        assert(lons.isEmpty());
        // assert(tags.isEmpty());
    }

}

void OsmPbfContext::decodeWay(protobuf::Message data)
{
    int64_t id = 0;
    protobuf::Message keys;
    protobuf::Message values;
    protobuf::Message nodes;
   
    const uint8_t* p = data.p;
    while (p < data.pEnd)
    {
        protobuf::Field field = protobuf::readField(p);
        switch (field)
        {
        case ELEMENT_ID:
            id = readVarint64(p);
            break;
        case ELEMENT_KEYS:
            keys = protobuf::readMessage(p);
            break;
        case ELEMENT_VALUES:
            values = protobuf::readMessage(p);
            break;
        case ELEMENT_INFO:
            protobuf::readMessage(p);
            break;
        case WAY_NODES:
            nodes = protobuf::readMessage(p);
            break;
        default:
            protobuf::skipEntity(p, field);
            break;
        }
    }
    /*
    TODO
    way(id, keyBuf == null ? EMPTY_TAGS :
        new KeyValueTags(strings, keyBuf, valueBuf),
        new Nodes(nodesBuf == null ? PbfBuffer.EMPTY : nodesBuf));
    */
}

void OsmPbfContext::decodeRelation(protobuf::Message data)
{
    int64_t id = 0;
    protobuf::Message keys;
    protobuf::Message values;
    protobuf::Message roles;
    protobuf::Message memberIds;
    protobuf::Message memberTypes;

    const uint8_t* p = data.p;
    while (p < data.pEnd)
    {
        protobuf::Field field = protobuf::readField(p);
        switch (field)
        {
        case ELEMENT_ID:
            id = readVarint64(p);
            break;
        case ELEMENT_KEYS:
            keys = protobuf::readMessage(p);
            break;
        case ELEMENT_VALUES:
            values = protobuf::readMessage(p);
            break;
        case ELEMENT_INFO:
            protobuf::readMessage(p);
            break;
        case RELATION_MEMBER_ROLES:
            roles = protobuf::readMessage(p);
            break;
        case RELATION_MEMBER_IDS:
            memberIds = protobuf::readMessage(p);
            break;
        case RELATION_MEMBER_TYPES:
            memberTypes = protobuf::readMessage(p);
            break;
        default:
            protobuf::skipEntity(p, field);
            break;
        }
    }
    /*
    TODO
    relation(id, keyBuf == null ? EMPTY_TAGS :
        new KeyValueTags(strings, keyBuf, valueBuf),
        new Members(strings, memberRolesBuf, memberIdsBuf, memberTypesBuf));
    */
}
