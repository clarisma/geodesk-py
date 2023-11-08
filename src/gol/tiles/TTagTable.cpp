#include "TTagTable.h"

void TTagTable::read(pointer ppTags)
{
	int rel = ppTags.getInt();
	int hasLocalTags = rel & 1;
	pointer pTable = ppTags + (rel ^ hasLocalTags);

}
