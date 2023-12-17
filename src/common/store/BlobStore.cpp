// Copyright (c) 2023 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#include "BlobStore.h"
#include <common/util/Bits.h>

uint64_t BlobStore::getLocalCreationTimestamp() const 
{
	return pointer(mainMapping()).getUnsignedLong(LOCAL_CREATION_TIMESTAMP_OFS);
}

uint64_t BlobStore::getTrueSize() const 
{
	int totalPages = pointer(mainMapping()).getInt(TOTAL_PAGES_OFS);
	// TODO: totalPages should really be unsigned, but we treat it as int
	// to stay compatible with the Java implementation
	// TODO: A FeatureStore cannot have more than 2^31 pages because 
	// of the marker bit in the tile index
	return static_cast<uint64_t>(totalPages) << pageSizeShift_;
}

void BlobStore::verifyHeader() const 
{
	if (pointer(mainMapping()).getUnsignedInt() != MAGIC)
	{
		error("Not a BlobStore file");
	}
}


void BlobStore::createStore()
{
	// TODO
}


void BlobStore::initialize()
{
	// Do nothing (TOOD: may change)
}

uint32_t BlobStore::pagesForPayloadSize(uint32_t payloadSize) const
{
    assert(payloadSize <= SEGMENT_LENGTH - BLOB_HEADER_SIZE);
    return (payloadSize + (1 << pageSizeShift_) + BLOB_HEADER_SIZE - 1) >> pageSizeShift_;
}


uint32_t BlobStore::Transaction::alloc(uint32_t payloadSize)
{
    assert (payloadSize <= SEGMENT_LENGTH - BLOB_HEADER_SIZE);
    // int precedingBlobFreeFlag = 0;
    uint32_t requiredPages = store()->pagesForPayloadSize(payloadSize);
    Header* rootBlock = reinterpret_cast<Header*>(getBlock(0));
    uint32_t trunkRanges = rootBlock->trunkFreeTableRanges;
    if (trunkRanges != 0)
    {
        // If there are free blobs, check if there is one large enough
        // to accommodate the requested size

        // The first slot in the trunk FT worth checking
        uint32_t trunkSlot = (requiredPages - 1) / 512;

        // The first slot in the leaf FT to check (for subsequent
        // leaf FTs, we need to start at 0
        uint32_t leafSlot = (requiredPages - 1) % 512;
        int trunkOfs = TRUNK_FREE_TABLE_OFS + trunkSlot * 4;
        int trunkEnd = (trunkOfs & 0xffff'ffc0) + 64;

        // We don't care about ranges that cover page counts that are less
        // than the number of pages we require, so shift off those bits
        trunkRanges >>= trunkSlot / 16;

        for (;;)
        {
            if ((trunkRanges & 1) == 0)
            {
                // There are no free blobs in the target range, so let's
                // check free blobs in the next-larger range; if there
                // aren't any, we quit

                if (trunkRanges == 0) break;

                // If a range does not contain any free blobs, no need
                // to check each of its 16 entries individually. The number
                // of zero bits tells us how many ranges we can skip

                int rangesToSkip = Bits::countTrailingZerosInNonZero(trunkRanges);
                trunkEnd += rangesToSkip * 64;
                trunkOfs = trunkEnd - 64;

                // for any range other than the first, we check all leaf slots

                leafSlot = 0;
            }
            assert(trunkOfs < TRUNK_FREE_TABLE_OFS + FREE_TABLE_LEN);

            for (; trunkOfs < trunkEnd; trunkOfs += 4)
            {
                uint32_t leafTableBlob = rootBlock.getUnsignedInt(trunkOfs);
                if (leafTableBlob == 0) continue;

                // There are one or more free blobs within the
                // 512-page size range indicated by trunkOfs

                pointer leafBlock = getBlockOfPage(leafTableBlob);
                int leafRanges = leafBlock.getInt(LEAF_FT_RANGE_BITS_OFS);
                int leafOfs = LEAF_FREE_TABLE_OFS + leafSlot * 4;
                int leafEnd = (leafOfs & 0xffff'ffc0) + 64;

                /*
                assert(leafBlock.getInt(0) & FREE_BLOB_FLAG) != 0 :
                String.format("Leaf FB blob %d must be a free blob",
                    leafTableBlob);
                */

                leafRanges >>= leafSlot / 16;

                for (; ; )
                {
                    if ((leafRanges & 1) == 0)
                    {
                        if (leafRanges == 0) break;
                        int rangesToSkip = Bits::countTrailingZerosInNonZero(leafRanges);
                        leafEnd += rangesToSkip * 64;
                        leafOfs = leafEnd - 64;
                    }
                    for (; leafOfs < leafEnd; leafOfs += 4)
                    {
                        uint32_t freeBlob = leafBlock.getUnsignedInt(leafOfs);
                        if (freeBlob == 0) continue;

                        // Found a free blob of sufficient size

                        int freePages = ((trunkOfs - TRUNK_FREE_TABLE_OFS) << 7) +
                            ((leafOfs - LEAF_FREE_TABLE_OFS) >> 2) + 1;
                        if (freeBlob == leafTableBlob)
                        {
                            // If the free blob is the same blob that holds
                            // the leaf free-table, check if there is another
                            // free blob of the same size

                            // log.debug("    Candidate free blob {} holds leaf FT", freeBlob);

                            int nextFreeBlob = leafBlock.getInt(NEXT_FREE_BLOB_OFS);
                            if (nextFreeBlob != 0)
                            {
                                // If so, we'll use that blob instead

                                // log.debug("    Using next free blob at {}", nextFreeBlob);
                                freeBlob = nextFreeBlob;
                            }
                        }

                        // TODO!!!!!
                        // TODO: bug: we need to relocate ft after we
                        //  add the remaining part
                        //  free blob is last of size, remaining is in same leaf FT
                        //  won't move the FT, FT ends up in allocated portion
                        //  OR: remove entire blob first,then add remaining?
                        //  Solution: reverse sequence: remove whole blob first,
                        //  then add back the remaining part

                        pointer freeBlock = getBlockOfPage(freeBlob);
                        uint32_t rawFreeBlobPayloadSize =
                            freeBlock.getUnsignedInt(BLOB_PAYLOAD_SIZE_OFS);
                        assert(rawFreeBlobPayloadSize & FREE_BLOB_FLAG);
                        uint32_t freeBlobPayloadSize = rawFreeBlobPayloadSize & PAYLOAD_SIZE_MASK;
                        assert((rawFreeBlobPayloadSize + BLOB_HEADER_SIZE) >> 
                            store()->pageSizeShift_ == freePages);
                        assert (freePages >= requiredPages);
                        removeFreeBlob(freeBlock);

                        if (freeBlob == leafTableBlob)
                        {
                            // We need to move the freetable to another free blob
                            // (If it is no longer needed, this is a no-op;
                            // removeFreeBlob has already set the trunk slot to 0)
                            // TODO: consolidate with removeFreeBlob?
                            //  We only separate this step because in freeBlob
                            //  we are potentially removing preceding/following
                            //  blob of same size range, which means we'd have
                            //  to move FT twice

                            uint32_t newLeafBlob = relocateFreeTable(freeBlob, freePages);
                            if (newLeafBlob != 0)
                            {
                                // log.debug("    Moved leaf FT to {}", newLeafBlob);
                                assert (rootBlock.getUnsignedInt(trunkOfs) == newLeafBlob);
                            }
                            else
                            {
                                // log.debug("    Leaf FT no longer needed");
                                assert (rootBlock.getUnsignedInt(trunkOfs) == 0);
                            }
                        }

                        if (freePages > requiredPages)
                        {
                            // If the free blob is larger than needed, mark the
                            // remainder as free and add it to its respective free list;
                            // we always do this before we remove the reused blob, since
                            // we may needlessly remove and reallocate the free table
                            // if the reused is the last blob in the table, but the
                            // remainder is in the same 512-page range

                            // We won't need to touch the preceding-free flag of the
                            // successor blob, since it is already set

                            addFreeBlob(freeBlob + requiredPages, freePages - requiredPages, 0);
                        }
                        uint8_t* nextBlock = getBlockOfPage(freeBlob + freePages);
                        *reinterpret_cast<uint32_t*>(nextBlock) = freePages - requiredPages;


                        // TODO: freeBlock.putInt(0, payloadSize);
                        // debugCheckRootFT();
                        return freeBlob;
                    }
                    leafRanges >>= 1;
                    leafEnd += 64;
                }
                leafSlot = 0;
            }

            // Check the next range

            trunkRanges >>= 1;
            trunkEnd += 64;
        }
    }

    // If we weren't able to find a suitable free blob,
    // we'll grow the store

    uint32_t totalPages = rootBlock.getUnsignedInt(TOTAL_PAGES_OFS);
    uint32_t pagesPerSegment = SEGMENT_LENGTH >> store()->pageSizeShift_;
    int remainingPages = pagesPerSegment - (totalPages & (pagesPerSegment - 1));
    if (remainingPages < requiredPages)
    {
        // If the blob won't fit into the current segment, we'll
        // mark the remaining space as a free blob, and allocate
        // the blob in a fresh segment

        addFreeBlob(totalPages, remainingPages, 0);
        totalPages += remainingPages;

        // In this case, we'll need to set the preceding-free flag of the
        // allocated blob

        precedingBlobFreeFlag = PRECEDING_BLOB_FREE_FLAG;
    }
    rootBlock.putInt(TOTAL_PAGES_OFS, totalPages + requiredPages);

    // TODO: no need to journal the blob's header block if it is in
    //  virgin space
    //  But: need to mark the segment as dirty, so it can be forced
    ByteBuffer newBlock = getBlockOfPage(totalPages);
    newBlock.putInt(0, size | precedingBlobFreeFlag);
    // debugCheckRootFT();
    return totalPages;
}
