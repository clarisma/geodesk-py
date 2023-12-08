#pragma once
#include <string_view>
#include <common/thread/TaskEngine.h>
#include <common/util/protobuf.h>
#include <cstdarg> // For va_list, va_start, va_end

class OsmPbfReader;

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

class WorkContext
{
public:
	WorkContext(void* reader) :
		reader_(reinterpret_cast<OsmPbfReader*>(reader))
	{}

	void processTask(OsmPbfBlock& task);
	void* engine() const { return reader_;};

private:
	void readStringTable(protobuf::Message strings);
	void decodePrimitiveGroup(protobuf::Message group);
	void decodeDenseNodes(protobuf::Message data);
	void decodeWay(protobuf::Message data);
	void decodeRelation(protobuf::Message data);

protected:
	OsmPbfReader* reader_;
	std::vector<std::string_view> strings_;
	std::vector<protobuf::Message> groups_;
	int64_t latOffset_;
	int64_t lonOffset_;
	uint32_t granularity_;
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


class OsmPbfOutputTask
{
};


class OsmPbfReader : public TaskEngine<OsmPbfReader, WorkContext, OsmPbfBlock, OsmPbfOutputTask>
{
public:
	OsmPbfReader(int numberOfThreads) :
		TaskEngine(numberOfThreads)
	{
	}

	void read(const char* fileName);

	void processTask(OsmPbfOutputTask& task)
	{
		// do nothing
	}

protected:
	

private:
	static UncompressedBlock uncompressBlock(const OsmPbfBlock& block);
	void decodeHeaderBlock(const OsmPbfBlock& block);

	friend class WorkContext;
};
