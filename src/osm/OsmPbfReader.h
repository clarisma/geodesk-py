#pragma once
#include <common/thread/TaskEngine.h>

class OsmPbfReader;

class OsmPbfBlock
{
	const uint8_t* data;
	uint32_t dataSize;
	uint32_t blockSize;
};

class WorkContext
{
public:
	WorkContext(void* reader) :
		reader_(reinterpret_cast<OsmPbfReader*>(reader))
	{}

	void processTask(OsmPbfBlock& task);
	void* engine() const { return reader_;};

protected:
	OsmPbfReader* reader_;
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

};
