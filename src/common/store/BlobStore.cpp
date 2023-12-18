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
    Header* rootBlock = getRootBlock();
    uint32_t trunkRanges = rootBlock->trunkFreeTableRanges;
    if (trunkRanges != 0)
    {
        // If there are free blobs, check if there is one large enough
        // to accommodate the requested size

        // The first slot in the trunk FT worth checking
        uint32_t trunkSlot = (requiredPages - 1) / 512;
        uint32_t trunkSlotEnd = (trunkSlot & 0x1f0) + 16;

        // The first slot in the leaf FT to check (for subsequent
        // leaf FTs, we need to start at 0
        uint32_t leafSlot = (requiredPages - 1) % 512;
        // int trunkOfs = TRUNK_FREE_TABLE_OFS + trunkSlot * 4;
        // int trunkEnd = (trunkOfs & 0xffff'ffc0) + 64;

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
                trunkSlotEnd += rangesToSkip * 16;
                trunkSlot = trunkSlotEnd - 16;
                // trunkEnd += rangesToSkip * 64;
                // trunkOfs = trunkEnd - 64;

                // for any range other than the first, we check all leaf slots

                leafSlot = 0;
            }
            // assert(trunkOfs < TRUNK_FREE_TABLE_OFS + FREE_TABLE_LEN);

            for (; trunkSlot < trunkSlotEnd; trunkSlot++)
            {
                uint32_t leafTableBlob = rootBlock->trunkFreeTable[trunkSlot];
                if (leafTableBlob == 0) continue;

                // There are one or more free blobs within the
                // 512-page size range indicated by trunkOfs

                Blob* leafBlock = getBlobBlock(leafTableBlob);
                int leafRanges = leafBlock->leafFreeTableRanges;
                // int leafOfs = LEAF_FREE_TABLE_OFS + leafSlot * 4;
                // int leafEnd = (leafOfs & 0xffff'ffc0) + 64;
                uint32_t leafSlotEnd = (leafSlot & 0x1f0) + 16;
                assert(leafBlock->isFree);

                leafRanges >>= leafSlot / 16;

                for (; ; )
                {
                    if ((leafRanges & 1) == 0)
                    {
                        if (leafRanges == 0) break;
                        int rangesToSkip = Bits::countTrailingZerosInNonZero(leafRanges);
                        leafSlotEnd += rangesToSkip * 16;
                        leafSlot = leafSlotEnd - 16;
                        // leafEnd += rangesToSkip * 64;
                        // leafOfs = leafEnd - 64;
                    }
                    for (; leafSlot < leafSlotEnd; leafSlot++)
                    {
                        uint32_t freeBlob = leafBlock->leafFreeTable[leafSlot];
                        if (freeBlob == 0) continue;

                        // Found a free blob of sufficient size

                        uint32_t freePages = trunkSlot * 512 + leafSlot + 1;
                        if (freeBlob == leafTableBlob)
                        {
                            // If the free blob is the same blob that holds
                            // the leaf free-table, check if there is another
                            // free blob of the same size

                            // log.debug("    Candidate free blob {} holds leaf FT", freeBlob);

                            uint32_t nextFreeBlob = leafBlock->nextFreeBlob;
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

                        Blob* freeBlock = getBlobBlock(freeBlob);
                        assert(freeBlock->isFree);
                        uint32_t freeBlobPayloadSize = freeBlock->payloadSize;
                        assert((freeBlobPayloadSize + BLOB_HEADER_SIZE) >> 
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
                                assert (rootBlock->trunkFreeTable[trunkSlot] == newLeafBlob);
                            }
                            else
                            {
                                // log.debug("    Leaf FT no longer needed");
                                assert (rootBlock->trunkFreeTable[trunkSlot] == 0);
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
                        Blob* nextBlock = getBlobBlock(freeBlob + freePages);
                        nextBlock->precedingFreeBlobPages = freePages - requiredPages;


                        // TODO: freeBlock.putInt(0, payloadSize);
                        // debugCheckRootFT();
                        return freeBlob;
                    }
                    leafRanges >>= 1;
                    leafSlotEnd += 16;
                }
                leafSlot = 0;
            }

            // Check the next range

            trunkRanges >>= 1;
            trunkSlotEnd += 16;
        }
    }

    // If we weren't able to find a suitable free blob,
    // we'll grow the store

    uint32_t totalPages = rootBlock->totalPageCount;
    uint32_t pagesPerSegment = SEGMENT_LENGTH >> store()->pageSizeShift_;
    int remainingPages = pagesPerSegment - (totalPages & (pagesPerSegment - 1));
    uint32_t precedingFreePages = 0;
    if (remainingPages < requiredPages)
    {
        // If the blob won't fit into the current segment, we'll
        // mark the remaining space as a free blob, and allocate
        // the blob in a fresh segment

        addFreeBlob(totalPages, remainingPages, 0);
        totalPages += remainingPages;

        // In this case, we'll need to set the preceding-free flag of the
        // allocated blob

        precedingFreePages = remainingPages;
    }
    rootBlock->totalPageCount = totalPages + requiredPages;

    // TODO: no need to journal the blob's header block if it is in
    //  virgin space
    //  But: need to mark the segment as dirty, so it can be forced
    Blob* newBlock = getBlobBlock(totalPages);
    newBlock->precedingFreeBlobPages = precedingFreePages;
    newBlock->payloadSize = payloadSize;
    newBlock->isFree = false;
    // debugCheckRootFT();
    return totalPages;
}


void BlobStore::Transaction::removeFreeBlob(Blob* freeBlock)
{
    uint32_t prevBlob = freeBlock->prevFreeBlob;
    uint32_t nextBlob = freeBlock->nextFreeBlob;

    // Unlink the blob from its siblings

    if (nextBlob != 0)
    {
        Blob* nextBlock = getBlobBlock(nextBlob);
        nextBlock->prevFreeBlob = prevBlob;
    }
    if (prevBlob != 0)
    {
        Blob* prevBlock = getBlobBlock(prevBlob);
        prevBlock->nextFreeBlob = nextBlob;
        return;
    }

    // Determine the free blob that holds the leaf freetable
    // for free blobs of this size

    uint32_t payloadSize = freeBlock->payloadSize;
    int pages = (payloadSize + BLOB_HEADER_SIZE) >> store()->pageSizeShift_;
    int trunkSlot = (pages - 1) / 512;
    int leafSlot = (pages - 1) % 512;

    // log.debug("     Removing blob with {} pages", pages);

    Header* rootBlock = getRootBlock();
    uint32_t leafBlob = rootBlock->trunkFreeTable[trunkSlot];

    // If the leaf FT has already been dropped, there's nothing
    // left to do (TODO: this feels messy)
    // (If we don't check, we risk clobbering the root FT)
    // TODO: leafBlob should never be 0!

    assert (leafBlob != 0);

    // if(leafBlob == 0) return;

    // Set the next free blob as the first blob of its size

    Blob* leafBlock = getBlobBlock(leafBlob);
    leafBlock->leafFreeTable[leafSlot] = nextBlob;
    if (nextBlob != 0) return;

    // Check if there are any other free blobs in the same size range

    uint32_t leafRange = leafSlot / 16;
    assert (leafRange >= 0 && leafRange < 32);

    uint32_t startLeafSlot = leafRange * 16;
    uint32_t endLeafSlot = startLeafSlot + 16;
    for (uint32_t slot = startLeafSlot; slot<endLeafSlot; slot++)
    {
        if (leafBlock->leafFreeTable[slot] != 0) return;
    }

    // The range has no free blobs, clear its range bit

    leafBlock->leafFreeTableRanges &= ~(1 << leafRange); 
    if (leafBlock->leafFreeTableRanges != 0) return;

    // No ranges are in use, which means the leaf free table is
    // no longer required

    rootBlock->trunkFreeTable[trunkSlot] = 0;

    // Check if there are any other leaf tables in the same size range

    uint32_t trunkRange = trunkSlot / 16;
    assert (trunkRange >= 0 && trunkRange < 32);
    uint32_t startTrunkSlot = trunkRange * 16;
    uint32_t endTrunkSlot = startTrunkSlot + 16;
    for (uint32_t slot = startTrunkSlot; slot < endTrunkSlot; slot++)
    {
        if (rootBlock->trunkFreeTable[slot] != 0) return;
    }

    // The trunk range has no leaf tables, clear its range bit
    rootBlock->trunkFreeTableRanges &= ~(1 << trunkRange);
}
