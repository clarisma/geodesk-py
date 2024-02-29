#include "PileFile.h"

#include <cassert>
#include <common/util/Bits.h>

PileFile::PileFile()
{

}


void PileFile::create(const char* filename, uint32_t pileCount, uint32_t pageSize)
{
	assert(pileCount > 0 && pileCount < ((1 << 26) - 1));
	assert(pageSize >= 4096 && pageSize <= SEGMENT_LENGTH && Bits::bitCount(pageSize) == 1);
	open(filename, OpenMode::READ | OpenMode::WRITE | OpenMode::CREATE);
	Metadata* meta = metadata();
	pageSize_ = pageSize;
	meta->pageSizeShift = Bits::countTrailingZerosInNonZero(pageSize);
	meta->pileCount = pileCount;
	
	uint32_t entriesPerPage = pageSize / sizeof(IndexEntry);
	meta->pageCount = (pileCount + entriesPerPage) / entriesPerPage;
		// No need to use -1, because count includes the metadata (which 
		// has the same size as an index entry)
}


void PileFile::append(uint32_t pile, const uint8_t* data, size_t len)
{
	assert (pile > 0 && pile <= metadata()->pileCount);
	// assert baseMapping.limit() == (1 << 30) : String.format("Wrong buffer limit = %d", baseMapping.limit());
	IndexEntry* indexEntry = &metadata()->index[pile - 1];
	uint32_t lastPage = indexEntry->lastPage;
	uint64_t pileSize = indexEntry->grossSize;
	if (lastPage == 0)
	{
		// Pile does not yet exist
		lastPage = allocPage();
		pileSize = sizeof(PageHeader);
		indexEntry->firstPage = lastPage;
		indexEntry->lastPage = lastPage;
	}
	size_t lastPageUsedBytes = pileSize & (pageSize_-1);
	if (lastPageUsedBytes == 0) lastPageUsedBytes = pageSize_;
	size_t pageSpaceRemaining = pageSize_ - lastPageUsedBytes;
	Page* pPage = getPage(lastPage);
	uint8_t* pPageData = reinterpret_cast<uint8_t*>(pPage);
	if (pageSpaceRemaining >= len)
	{
		memcpy(pPageData + lastPageUsedBytes, data, len);
	}
	else
	{
		size_t remainingLen = len;
		for (;;)
		{
			memcpy(pPageData + lastPageUsedBytes, data, std::min(pageSpaceRemaining, remainingLen));
			data += pageSpaceRemaining;
			remainingLen -= pageSpaceRemaining;
			if (static_cast<int64_t>(remainingLen) <= 0) break;
			lastPage = allocPage();
			pPage->nextPage = lastPage;
			pageSpaceRemaining = pageSize_ - sizeof(PageHeader);
			lastPageUsedBytes = sizeof(PageHeader);
			pileSize += sizeof(PageHeader);
			pPage = getPage(lastPage);
			uint8_t* pPageData = reinterpret_cast<uint8_t*>(pPage);
		}
		indexEntry->lastPage = lastPage;
	}
	indexEntry->grossSize = pileSize + len;
}


PileFile::Data PileFile::load(uint32_t pile)
{
	assert (pile > 0 && pile <= metadata()->pileCount);
	IndexEntry* indexEntry = &metadata()->index[pile - 1];
	uint32_t page = indexEntry->firstPage;
	if (page == 0)
	{
		uint8_t* p = new uint8_t[1];
		return Data{p, p};
	}
	uint64_t pileSize = indexEntry->grossSize;
	uint64_t numberOfPages = (pileSize + pageSize_ - 1) >> metadata()->pageSizeShift;
	uint64_t dataSize = pileSize - numberOfPages * sizeof(PageHeader);
	uint8_t* pStart = new uint8_t[dataSize];
	uint8_t* pEnd = pStart;
	uint64_t dataPerPage = pageSize_ - sizeof(PageHeader);
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
		uint64_t dataRead = std::min(dataSize, dataPerPage);
		memcpy(pEnd, &pPage->data, dataRead);
		pEnd += dataRead;
		dataSize -= dataPerPage;
		// TODO: detect underflow = corrupt pile file
	}
	return Data{ pStart, pEnd };
}
