#pragma once
// #include <unordered_map>
#include <common/alloc/Arena.h>
#include "TileReader.h"
#include "TString.h"
#include "TTagTable.h"

class TileCompiler : public TileReader<TileCompiler>
{
public:
	TileCompiler();

private:
	void readTile(pointer pTile);
	void readNode(NodeRef node);
	void readWay(WayRef way);
	void readRelation(RelationRef relation);
	TString* readString(pointer p);
	TTagTable* readTagTable(pointer pTagged);

	int32_t currentLocation(pointer p)
	{
		return static_cast<int32_t>(pCurrentTile_ - p);
		// We use negative values to indicate old location
	}

	Arena arena_;
	LookupByLocation elementsByLocation_;
	ElementDeduplicator<TString> strings_;
	ElementDeduplicator<TTagTable> tagTables_;
	uint8_t* pCurrentTile_;
	uint32_t currentTileSize_;
	int tagTableCount_;
	int stringCount_;
};
