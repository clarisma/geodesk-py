// Copyright (c) 2023 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include "Store.h"
#include <common/util/pointer.h>

class BlobStore : public Store
{
public:
	void prefetchBlob(const void* pBlob)
	{
		uint32_t size = pointer(pBlob).getUnsignedInt() & 0x3fff'ffff;
		prefetch(pBlob, size);
	}

protected:
	static const uint32_t MAGIC = 0x7ADA0BB1;
	static const uint32_t VERSION = 1'000'000;
	
	static const int VERSION_OFS = 4;
	static const int LOCAL_CREATION_TIMESTAMP_OFS = 8;
	static const int TOTAL_PAGES_OFS = 16;

	pointer pagePointer(uint32_t page)
	{
		return pointer(data(static_cast<uint64_t>(page) << pageSizeShift_));
	}

	void createStore() override;
	void verifyHeader() const override;
	void initialize() override;

	uint64_t getLocalCreationTimestamp() const override;
	uint64_t getTrueSize() const override;
	
	uint32_t pageSizeShift_ = 12;	// TODO: default 4KB page
};


