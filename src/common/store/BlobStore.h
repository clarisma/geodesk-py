// Copyright (c) 2023 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include "Store.h"
#include <unordered_set>
#include <common/util/pointer.h>

class BlobStore : public Store
{
public:
	void prefetchBlob(const void* pBlob)
	{
		uint32_t size = pointer(pBlob).getUnsignedInt() & 0x3fff'ffff;
		prefetch(pBlob, size);
	}

	class Transaction : private Store::Transaction
	{
	public:
		Transaction(BlobStore* store) :
			Store::Transaction(store)
		{
		}

		BlobStore* store() const { return reinterpret_cast<BlobStore*>(store_); }
		uint32_t alloc(uint32_t payloadSize);
		void free(uint32_t firstPage);

	private:
		uint8_t* getBlockOfPage(uint32_t page)
		{
			return getBlock(static_cast<uint64_t>(page) << store()->pageSizeShift_);
		}

		std::unordered_set<uint32_t> freedBlobs_;
	};

	uint8_t* translatePage(uint32_t page);

protected:
	static const uint32_t MAGIC = 0x7ADA0BB1;
	static const uint32_t VERSION = 1'000'000;

	static const int BLOB_HEADER_SIZE = 8;

	static const int BLOB_PAYLOAD_SIZE_OFS = 4;

	/**
	 * A bit mask that, when applied to a Blob's header word, yields
	 * the size of the Blob's payload (max. 1 GB - 8).
	 */
	static const uint32_t PAYLOAD_SIZE_MASK = 0x3fff'ffff;

	/**
	 * Flag to indicate that a Blob is free. Stored in the payload length
	 * field of the Blob Header
	 */
	static const uint32_t FREE_BLOB_FLAG = 0x8000'0000;

	static const int FREE_TABLE_LEN = 2048;
	
	static const int VERSION_OFS = 4;
	static const int LOCAL_CREATION_TIMESTAMP_OFS = 8;
	static const int TOTAL_PAGES_OFS = 16;		// TODO: will change
	static const int TRUNK_FT_RANGE_BITS_OFS = 52;

	/**
	 * Offset of the trunk free-table.
	 * (This offset must be evenly divisible by 64)
	 */
	static const int TRUNK_FREE_TABLE_OFS = 128;     // must be divisible by 64

	static const int PREV_FREE_BLOB_OFS = 8;
	static const int NEXT_FREE_BLOB_OFS = 12;
	static const int LEAF_FT_RANGE_BITS_OFS = 16;
	static const int LEAF_FREE_TABLE_OFS = 64;     // must be divisible by 64


	pointer pagePointer(uint32_t page)
	{
		return pointer(data(static_cast<uint64_t>(page) << pageSizeShift_));
	}

	void createStore() override;
	void verifyHeader() const override;
	void initialize() override;

	uint64_t getLocalCreationTimestamp() const override;
	uint64_t getTrueSize() const override;
	uint32_t pagesForPayloadSize(uint32_t payloadSize) const;
		
private:
	struct Header
	{
		uint32_t magic;
		uint16_t versionLow;
		uint16_t versionHigh;
		uint64_t creationtimestamp;
		uint8_t  guid[16];
		uint8_t  pageSize;
		uint32_t reserved : 24;
		uint32_t metadataSize;
		uint32_t propertiesPointer;
		uint32_t indexPointer;
		uint32_t totalPageCount;
		uint32_t trunkFreeTableRanges;
		uint32_t datasetVersion;
		uint32_t reserved2;
		uint8_t  subtypeData[64];
		uint32_t trunkFreeTable[512];
	};

	struct Blob
	{
		uint32_t precedingFreeBlobPages;
		uint32_t payloadSize : 30;
		bool     unused : 1;
		bool     isFree : 1;
		uint32_t prevFreeBlob;
		uint32_t nextFreeBlob;
		uint32_t leafFreeTableRanges;
	};

	uint32_t pageSizeShift_ = 12;	// TODO: default 4KB page
};


