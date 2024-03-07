#include "PileFile.h"

#include <cassert>
#include <common/util/Bits.h>
#include <common/util/Bytes.h>

// TODO: Implement open()!

PileFile::PileFile()
{

}


void PileFile::create(std::filesystem::path filePath, uint32_t pileCount, uint32_t pageSize)
{
	assert(pileCount > 0 && pileCount < MAX_PILE_COUNT);
	assert(pageSize >= 4096 && pageSize <= SEGMENT_LENGTH && Bytes::isPowerOf2(pageSize));
	if (std::filesystem::exists(filePath))
	{
		// Delete exisitng file so we start with a blank slate
		std::filesystem::remove(filePath);
	}

	open(filePath.string().c_str(), OpenMode::READ | OpenMode::WRITE | OpenMode::CREATE);
	Metadata* meta = metadata();
	pageSize_ = pageSize;
	meta->pageSizeShift = Bits::countTrailingZerosInNonZero(pageSize);
	meta->pileCount = pileCount;
	
	uint32_t entriesPerPage = pageSize / sizeof(IndexEntry);
	meta->pageCount = (pileCount + entriesPerPage) / entriesPerPage;
		// No need to use -1, because count includes the metadata (which 
		// has the same size as an index entry)
}


void PileFile::append(int pile, const uint8_t* data, size_t len)
{
	assert (pile > 0 && pile <= metadata()->pileCount);
	IndexEntry* indexEntry = &metadata()->index[pile - 1];
	uint32_t lastPage = indexEntry->lastPage;
	uint64_t pileSize = indexEntry->grossSize;
	if (lastPage == 0)
	{
		// Pile does not yet exist
		lastPage = allocPage();
		pileSize = PAGE_HEADER_SIZE;
		indexEntry->firstPage = lastPage;
		indexEntry->lastPage = lastPage;
	}
	uint32_t lastPageUsedBytes = pileSize & (pageSize_-1);
	if (lastPageUsedBytes == 0) lastPageUsedBytes = pageSize_;
	
	uint64_t remainingLen = len;
	for(;;)
	{
		Page* pPage = getPage(lastPage);
		uint64_t pageSpaceRemaining = pageSize_ - lastPageUsedBytes;
		if(pageSpaceRemaining > 0)
		{
			uint8_t* p = reinterpret_cast<uint8_t*>(pPage) + lastPageUsedBytes;
			uint64_t n = std::min(remainingLen, pageSpaceRemaining);
			memcpy(p, data, n);
			remainingLen -= n;
		}
		if (remainingLen == 0) break;
		lastPage = allocPage();
		pPage->nextPage = lastPage;
		pileSize += PAGE_HEADER_SIZE;
		lastPageUsedBytes = PAGE_HEADER_SIZE;
	}
	indexEntry->lastPage = lastPage;
	indexEntry->grossSize = pileSize + len;
}


void PileFile::load(int pile, ReusableBlock& block)
{
	assert (pile > 0 && pile <= metadata()->pileCount);
	IndexEntry* indexEntry = &metadata()->index[pile - 1];
	uint32_t page = indexEntry->firstPage;
	if (page == 0)
	{
		block.resize(0);
		return;
	}
	uint64_t pileSize = indexEntry->grossSize;
	uint64_t numberOfPages = (pileSize + pageSize_ - 1) >> metadata()->pageSizeShift;
	uint64_t dataSize = pileSize - numberOfPages * PAGE_HEADER_SIZE;
	block.resize(dataSize);
	uint8_t* pStart = block.data();
	uint8_t* pEnd = pStart;
	uint64_t dataPerPage = pageSize_ - PAGE_HEADER_SIZE;
	uint64_t remainingLen = dataSize;
	while (page != 0)
	{
		/*
		if (dataSize <= 0)
		{
			throw new IOException(
				String.format(
					"Corrupt PileFile, Pile %d has more pages than " +
					"actual data (Pile size = %d, next page = %d)",
					pile, pileSize, page));
		}
		*/

		const Page* pPage = getPage(page);
		uint64_t n = std::min(remainingLen, dataPerPage);
		page = pPage->nextPage;
		memcpy(pEnd, &pPage->data, n);
		pEnd += n;
		remainingLen -= n;
		assert(remainingLen < dataSize);		// check for wraparound
	}
	assert(remainingLen == 0);
}
