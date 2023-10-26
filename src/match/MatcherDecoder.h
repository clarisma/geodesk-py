// Copyright (c) 2023 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include <cstdint>
#include <common/util/BufferWriter.h>

class FeatureStore;

class MatcherDecoder
{
public:
	MatcherDecoder(FeatureStore* store, BufferWriter& out, const uint16_t* pCode) :
		store_(store), out_(out), pCodeStart_(pCode), pLastInstruction_(pCode) {}

	void decode();
	
private:
	void writeAddress(const uint16_t* p, bool padded);
	void writeOpcodeStub(const uint16_t* p);
	void writeBranchingOp(const uint16_t* p);

	const uint16_t* pCodeStart_;
	const uint16_t* pLastInstruction_;
	BufferWriter& out_;
	FeatureStore* store_;
};
