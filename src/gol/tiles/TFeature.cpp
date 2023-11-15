#include "TFeature.h"

void TFeature::write(const TTile* tile, uint8_t* p) const
{
	int32_t* pInt;
	if (feature_.isNode())
	{
		memcpy(p, feature_.ptr() - 8, 16);
		pInt = reinterpret_cast<int32_t*>(p + 8);
	}
	else
	{
		memcpy(p, feature_.ptr() - 16, 24);
		pInt = reinterpret_cast<int32_t*>(p + 16);
	}
	*pInt = (*pInt & ~1) | (next() ? 0 : 1);
		// set the is_last flag bit 

	// TODO: encode tag-table pointer
	// TODO: encode body pointer
}
