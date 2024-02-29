#include "PileWriter.h"
#include <common/store/PileFile.h>

void PileSet::writeTo(PileFile& file)
{
	Pile* pile = firstPile_;
	while (pile)
	{
		uint32_t pileNumber = pile->number_;
		uint32_t payloadSize = pageSize_ - sizeof(Pile);
		Page* page = pile;
		for (;;)
		{
			const uint8_t* data = reinterpret_cast<const uint8_t*>(page) +
				pageSize_ - payloadSize;
			Page* nextPage = page->next_;
			if (!nextPage)
			{
				payloadSize -= pile->remaining_;
				file.append(pileNumber, data, payloadSize);
				break;
			}
			file.append(pileNumber, data, payloadSize);
			payloadSize = pageSize_ - sizeof(Page);
			page = nextPage;
		}
		pile = pile->nextPile_;
	}
}
