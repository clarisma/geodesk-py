// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include <common/util/TaskEngine.h>

/*
class OsmPbfReader;
struct OsmPbfReader::Context;
struct OsmPbfReader::Task;
*/

struct OsmPbfReaderContext
{

};

struct OsmPbfBlock
{
	OsmPbfBlock() {}

	OsmPbfBlock(const uint8_t* pData, uint32_t dataLength, uint32_t blockLength)
		: pData_(pData), dataLength_(dataLength), blockLength_(blockLength)
	{
	}

	const uint8_t* pData_;
	uint32_t dataLength_;
	uint32_t blockLength_;
};


class OsmPbfReader : public TaskEngine<OsmPbfReaderContext, OsmPbfBlock>
{
public:
	OsmPbfReader(int maxThreads, int queueSize);
	void read(const char* fileName);

protected:
	void process(OsmPbfReaderContext& context, OsmPbfBlock task) override;
	int inflateBlock(const uint8_t* pZipped, uint32_t zippedLen, 
		uint8_t* pUnzipped, uint32_t unzippedLen);
	void decodeBlock(const uint8_t* pData, uint32_t len);
};
