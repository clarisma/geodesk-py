// Copyright (c) 2023 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#include "BlobStore.h"
#include <common/util/Bits.h>

uint64_t BlobStore::getLocalCreationTimestamp() const 
{
	return getRoot()->creationTimestamp;
}

uint64_t BlobStore::getTrueSize() const 
{
    return static_cast<uint64_t>(getRoot()->totalPageCount) << pageSizeShift_;
}

void BlobStore::verifyHeader() const 
{
    const Header* root = getRoot();
	if (root->magic != MAGIC)
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
                PageNum leafTableBlob = rootBlock->trunkFreeTable[trunkSlot];
                if (leafTableBlob == 0) continue;

                // There are one or more free blobs within the
                // 512-page size range indicated by trunkOfs

                Blob* leafBlock = getBlobBlock(leafTableBlob);
                int leafRanges = leafBlock->leafFreeTableRanges;
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
                    }
                    for (; leafSlot < leafSlotEnd; leafSlot++)
                    {
                        PageNum freeBlob = leafBlock->leafFreeTable[leafSlot];
                        if (freeBlob == 0) continue;

                        // Found a free blob of sufficient size

                        uint32_t freePages = trunkSlot * 512 + leafSlot + 1;
                        if (freeBlob == leafTableBlob)
                        {
                            // If the free blob is the same blob that holds
                            // the leaf free-table, check if there is another
                            // free blob of the same size

                            // log.debug("    Candidate free blob {} holds leaf FT", freeBlob);

                            PageNum nextFreeBlob = leafBlock->nextFreeBlob;
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

                            PageNum newLeafBlob = relocateFreeTable(freeBlob, freePages);
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
    PageNum prevBlob = freeBlock->prevFreeBlob;
    PageNum nextBlob = freeBlock->nextFreeBlob;

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
    PageNum leafBlob = rootBlock->trunkFreeTable[trunkSlot];

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


/**
 * Adds a blob to the freetable, and sets its size, header flags and trailer.
 *
 * This method does not affect the PRECEDING_BLOB_FREE_FLAG of the successor blob; it is the responsibility of the
 * caller to set the flag, if necessary.
 *
 * @param firstPage the first page of the blob
 * @param pages     the number of pages of this blob
 * @param freeFlags PRECEDING_BLOB_FREE_FLAG or 0
 */
void BlobStore::Transaction::addFreeBlob(PageNum firstPage, uint32_t pages, uint32_t precedingFreePages)
{
    Blob* block = getBlobBlock(firstPage);
    block->precedingFreeBlobPages = precedingFreePages;
    block->payloadSize = (pages << store()->pageSizeShift_) - BLOB_HEADER_SIZE;
    block->isFree = true;
    block->prevFreeBlob = 0;
    Header* rootBlock = getRootBlock();
    uint32_t trunkSlot = (pages - 1) / 512;
    PageNum leafBlob = rootBlock->trunkFreeTable[trunkSlot];
    Blob* leafBlock;
    if (leafBlob == 0)
    {
        // If no local free table exists for the size range of this blob,
        // this blob becomes the local free table

        block->leafFreeTableRanges = 0;
        memset(block->leafFreeTable, 0, sizeof(block->leafFreeTable));
        rootBlock->trunkFreeTableRanges |= 1 << (trunkSlot / 16);
        rootBlock->trunkFreeTable[trunkSlot] = firstPage;
        leafBlock = block;

        // Log.debug("  Free blob %d: Created leaf FT for %d pages", firstPage, pages);
    }
    else
    {
        leafBlock = getBlobBlock(leafBlob);
    }

    // Determine the slot in the leaf freetable where this
    // free blob will be placed

    uint32_t leafSlot = (pages - 1) % 512;
    PageNum nextBlob = leafBlock->leafFreeTable[leafSlot];
    if (nextBlob != 0)
    {
        // If a free blob of the same size exists already,
        // chain this blob as a sibling

        Blob* nextBlock = getBlobBlock(nextBlob);
        nextBlock->prevFreeBlob = firstPage;
    }
    block->nextFreeBlob = nextBlob;

    // Enter this free blob into the size-specific slot
    // in the leaf freetable, and set the range bit

    leafBlock->leafFreeTable[leafSlot] = firstPage;
    leafBlock->leafFreeTableRanges |= 1 << (leafSlot / 16);
}


/**
 * Deallocates a blob. Any adjacent free blobs are coalesced, 
 * provided that they are located in the same 1-GB segment.
 *
 * @param firstPage the first page of the blob
 */
void BlobStore::Transaction::free(PageNum firstPage)
{
    Header* rootBlock = getRootBlock();
    Blob* block = getBlobBlock(firstPage);
    
    assert(block->isFree);
    /*
    if (freeFlag != 0)
    {
        throw new StoreException(
            "Attempt to free blob that is already marked as free", path());
    }
    */

    uint32_t totalPages = rootBlock->totalPageCount;

    uint32_t pages = store()->pagesForPayloadSize(block->payloadSize);
    PageNum prevBlob = 0;
    PageNum nextBlob = firstPage + pages;
    uint32_t prevPages = block->precedingFreeBlobPages;
    uint32_t nextPages = 0;

    if (prevPages && !isFirstPageOfSegment(firstPage))
    {
        // If there is another free blob preceding this one,
        // and it is in the same segment, coalesce it

        prevBlob = firstPage - prevPages;
        Blob* prevBlock = getBlobBlock(prevBlob);

        // TODO: check:    
        // The preceding free blob could itself have a preceding free blob
        // (not coalesced because it is located in different segment),
        // so we preserve the preceding_free_flag

        removeFreeBlob(prevBlock);
        block = prevBlock;
        // log.debug("    Coalescing preceding blob at {} ({} pages})", prevBlob, prevPages);
    }

    if (nextBlob < totalPages && !isFirstPageOfSegment(nextBlob))
    {
        // There is another blob following this one,
        // and it is in the same segment

        Blob* nextBlock = getBlobBlock(nextBlob);
        if (nextBlock->isFree)
        {
            // The next blob is free, coalesce it

            nextPages = store()->pagesForPayloadSize(nextBlock->payloadSize);
            removeFreeBlob(nextBlock);

            // log.debug("    Coalescing following blob at {} ({} pages})", nextBlob, nextPages);
        }
    }

    if (prevPages != 0)
    {
        uint32_t trunkSlot = (prevPages - 1) / 512;
        PageNum prevFreeTableBlob = rootBlock->trunkFreeTable[trunkSlot];
        if (prevFreeTableBlob == prevBlob)
        {
            relocateFreeTable(prevFreeTableBlob, prevPages);
        }
    }
    if (nextPages != 0)
    {
        uint32_t trunkSlot = (nextPages - 1) / 512;
        PageNum nextFreeTableBlob = rootBlock->trunkFreeTable[trunkSlot];
        if (nextFreeTableBlob == nextBlob)
        {
            relocateFreeTable(nextFreeTableBlob, nextPages);
        }
    }

    pages += prevPages + nextPages;
    firstPage -= prevPages;

    /*
    if(pages == 262144)
    {
        Log.debug("Freeing 1-GB blob (First page = %d @ %X)...",
            firstPage, (long)firstPage << pageSizeShift);
    }
     */

    if (firstPage + pages == totalPages)
    {
        // If the free blob is located at the end of the file, reduce
        // the total page count (effectively truncating the store)

        totalPages = firstPage;
        while (block->precedingFreeBlobPages)
        {
            // If the preceding blob is free, that means it
            // resides in the preceding 1-GB segment (since we cannot
            // coalesce across segment boundaries). Remove this blob
            // from its freetable and trim it off. If this blob
            // occupies an entire segment, and its preceding blob is
            // free as well, keep trimming

            // Log.debug("Trimming across segment boundary...");

            prevPages = block->precedingFreeBlobPages;
            totalPages -= prevPages;
            prevBlob = totalPages;
            block = getBlobBlock(prevBlob);
            removeFreeBlob(block);

            // Move freetable, if necessary

            uint32_t trunkSlot = (prevPages - 1) / 512;
            PageNum prevFreeTableBlob = rootBlock->trunkFreeTable[trunkSlot];
            if (prevFreeTableBlob == prevBlob)
            {
                // Log.debug("Relocating free table for %d pages", prevPages);
                relocateFreeTable(prevBlob, prevPages);
            }

            if (!isFirstPageOfSegment(totalPages)) break;
        }
        rootBlock->totalPageCount = totalPages;

        // Log.debug("Truncated store to %d pages.", totalPages);
    }
    else
    {
        // Blob is not at end of file, add it to the free table

        addFreeBlob(firstPage, pages, block->precedingFreeBlobPages);
        Blob* nextBlock = getBlobBlock(firstPage + pages);
        nextBlock->precedingFreeBlobPages = pages;
    }
}
