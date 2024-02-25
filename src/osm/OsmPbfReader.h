// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once
#include <string_view>
#include <common/io/File.h>
#include <common/thread/ProgressReporter.h>
#include <common/thread/TaskEngine.h>
#include <common/util/Bytes.h>
#include <common/util/log.h>
#include <common/util/protobuf.h>
#include <cstdarg> // For va_list, va_start, va_end
#include <fstream>
#include <iostream>
#include <zlib.h>

/*
template <typename Derived, typename WorkContext, typename OutputTask>
class OsmPbfReader;
*/

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

class OsmPbfException : public std::runtime_error
{
public:
	explicit OsmPbfException(const char* format, ...)
		: std::runtime_error(formatMessage(format))
	{
	}

private:
	static std::string formatMessage(const char* format, ...)
	{
		char buf[256];
		va_list args;
		va_start(args, format);
		std::vsnprintf(buf, sizeof(buf), format, args);
		va_end(args);
		return std::string(buf);
	}
};

class OsmPbfBlock
{
public:
	const uint8_t* data;
	uint32_t dataSize;
	uint32_t blockSize;
};

class UncompressedBlock
{
public:
	UncompressedBlock() :
		data(nullptr),
		dataSize(0),
		toDelete(nullptr)
	{
	}

	UncompressedBlock(UncompressedBlock&& other)
	{
		data = other.data;
		dataSize = other.dataSize;
		toDelete = other.toDelete;
		other.toDelete = nullptr;
	}

	~UncompressedBlock() 
	{ 
		if(toDelete) delete[] toDelete; 
	}

	const uint8_t* data;
	uint32_t dataSize;
	const uint8_t* toDelete;
};

/**
 * stringTable()
 * startBlock()
 * endBlock()
 * afterTasks()
 */
template <typename Derived, typename Reader>
class OsmPbfContext
{
public:
	OsmPbfContext(void* reader) :
		reader_(reinterpret_cast<Reader*>(reader)),
		blockBytesProcessed_(0),
		latOffset_(0),
		lonOffset_(0),
		granularity_(100)
	{
	}

	Reader* reader() const { return reader_; };

	void processTask(OsmPbfBlock& block)
	{
		UncompressedBlock uncompressed = Reader::uncompressBlock(block);
		const uint8_t* p = uncompressed.data;
		const uint8_t* pEnd = p + uncompressed.dataSize;

		blockBytesProcessed_ += block.blockSize;
		self()->startBlock();

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
				// assert(strings_.empty());
				self()->stringTable(protobuf::readMessage(p));
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
		groups_.clear();
		self()->endBlock();
	}

	void afterTasks()	// CRTP override
	{
	}

	void harvestResults()	// CRTP override
	{
	}

protected:
	uint64_t blockBytesProcessed() const { return blockBytesProcessed_; }
	void resetBlockBytesProcessed() { blockBytesProcessed_ = 0; }
	/*
	const uint8_t* string(uint32_t index) const
	{
		return strings_[index];		// TODO: bounds check
	}
	*/

	void stringTable(protobuf::Message strings)  // CRTP override
	{
		// by default, do nothing
		/*
		// assert(_CrtCheckMemory());
		const uint8_t* p = strings.start;
		while (p < strings.end)
		{
			uint32_t marker = readVarint32(p);
			if (marker != STRINGTABLE_ENTRY)
			{
				throw OsmPbfException("Bad string table. Unexpected field: %d", marker);
			}
			const uint8_t* pString = p;
			p += readVarint32(p);
			strings_.push_back(pString);
		}
		assert(p == strings.end);
		*/
	}

	void startBlock()	// CRTP override
	{
	}

	void endBlock()		// CRTP override
	{
	}

private:
	Derived* self() { return reinterpret_cast<Derived*>(this); }

	void decodePrimitiveGroup(protobuf::Message group)
	{
		const uint8_t* p = group.start;
		while (p < group.end)
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
		assert(p == group.end);
	}

	void decodeDenseNodes(protobuf::Message data)
	{
		const uint8_t* p = data.start;
		protobuf::Message ids;
		protobuf::Message lats;
		protobuf::Message lons;
		protobuf::Message tags;

		while (p < data.end)
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
		assert(p == data.end);
		if (ids.start)
		{
			// TODO: check other messages are present
			const uint8_t* pId = ids.start;
			const uint8_t* pLat = lats.start;
			const uint8_t* pLon = lons.start;
			int64_t id = 0;
			int64_t lat = 0;
			int64_t lon = 0;

			// TODO: base iteration on lon or lat because 
			// Analyzer doesn't use ID, so the compiler may
			// simply eliminate the ID decoding
			while (pId < ids.end)
			{
				id += readSignedVarint64(pId);
				lat += readSignedVarint64(pLat);
				lon += readSignedVarint64(pLon);
				int64_t latInNanoDeg = (latOffset_ + (granularity_ * lat));
				int64_t lonInNanoDeg = (lonOffset_ + (granularity_ * lon));

				self()->node(id, static_cast<int32_t>(lonInNanoDeg / 100),
					static_cast<int32_t>(latInNanoDeg / 100), tags);
			}
			assert(pId == ids.end);
			assert(pLat == lats.end);
			assert(pLon == lons.end);
			// assert(tags.isEmpty());
		}
	}

	void decodeWay(protobuf::Message data)
	{
		int64_t id = 0;
		protobuf::Message keys;
		protobuf::Message values;
		protobuf::Message nodes;

		const uint8_t* p = data.start;
		while (p < data.end)
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
		self()->way(id, keys, values, nodes);
	}

	void decodeRelation(protobuf::Message data)
	{
		int64_t id = 0;
		protobuf::Message keys;
		protobuf::Message values;
		protobuf::Message roles;
		protobuf::Message memberIds;
		protobuf::Message memberTypes;

		const uint8_t* p = data.start;
		while (p < data.end)
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
		self()->relation(id, keys, values, roles, memberIds, memberTypes);
	}

	Reader* reader_;
	// std::vector<const uint8_t*> strings_;
	std::vector<protobuf::Message> groups_;
	int64_t latOffset_;
	int64_t lonOffset_;
	uint32_t granularity_;
	uint64_t blockBytesProcessed_;
};



class OsmPbfOutputTask
{
};

template <typename Derived, typename WorkContext, typename OutputTask>
class OsmPbfReader : public TaskEngine<Derived, WorkContext, OsmPbfBlock, OutputTask>
{
public:
	OsmPbfReader(int numberOfThreads) :
		TaskEngine<Derived, WorkContext, OsmPbfBlock, OutputTask>(numberOfThreads)
	{
	}

	void read(const char* fileName, ProgressReporter* progress=nullptr)
	{
		this->start();
		
		File file;
		file.open(fileName, File::OpenMode::READ);
		uint64_t fileSize = file.size();
		if (progress) progress->start(fileSize);

		size_t totalBytesRead = 0;
		while (totalBytesRead < fileSize)
		{
			// The header length (uint32) is in network byte order
			// TODO: Code below assumes native byte order is Little-Endian
			uint32_t rawHeaderLen;
			file.read(&rawHeaderLen, 4);
			uint32_t headerLen = Bytes::reverseByteOrder32(rawHeaderLen);

			if (headerLen > 256)
			{
				throw OsmPbfException("Excessive header length (%d)", headerLen);
			}

			uint8_t buf[256];
			file.read(buf, headerLen);
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
			file.read(data, dataLen);
			if (blockType == "OSMData")
			{
				// LOG("Block with %d bytes", block.blockSize);
				this->postWork(std::move(block));
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
		LOG("Waiting for threads to complete...");

		this->end();
		LOG("Done.");
	}

	void processTask(OsmPbfOutputTask& task)
	{
		// do nothing
	}

private:
	static UncompressedBlock uncompressBlock(const OsmPbfBlock& block)
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
		else if (pZipped)
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

	void decodeHeaderBlock(const OsmPbfBlock& block)
	{
		UncompressedBlock uncompressed = uncompressBlock(block);
		// TODO
	}

	friend class OsmPbfContext<WorkContext, Derived>;
};
