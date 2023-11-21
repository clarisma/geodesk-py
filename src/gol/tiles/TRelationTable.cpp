#include "TRelationTable.h"
#include "TTile.h"

void TRelationTable::write(const TTile& tile) const
{
	uint8_t* p = tile.newTileData() + location();
	TSharedElement::write(p);

	// TODO: adjust local-feature pointers
}
