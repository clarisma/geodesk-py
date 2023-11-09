#pragma once
// #include <unordered_map>
#include <common/alloc/Arena.h>
#include "TileReader.h"
#include "TFeature.h"
#include "TRelationTable.h"
#include "TString.h"
#include "TTagTable.h"

class TileCompiler : public TileReader<TileCompiler>
{
public:
	TileCompiler();
	void readTile(pointer pTile);

private:
	void readNode(NodeRef node);
	void readWay(WayRef way);
	void readRelation(RelationRef relation);
	TString* readString(pointer p);
	TTagTable* readTagTable(pointer pTagged);
	TRelationTable* readRelationTable(pointer p);

	int32_t currentLocation(pointer p)
	{
		return static_cast<int32_t>(pCurrentTile_ - p.asBytePointer());
		// We use negative values to indicate old location
	}

	void addFeatureToIndex(TFeature* feature)
	{
		elementsByLocation_.insert(feature);
		featuresById_.insert(feature);
	}

	void initTables(size_t tileSize);

	Arena arena_;
	LookupByLocation elementsByLocation_;
	LookupById featuresById_;
	ElementDeduplicator<TString> strings_;
	ElementDeduplicator<TTagTable> tagTables_;
	ElementDeduplicator<TRelationTable> relationTables_;
	uint8_t* pCurrentTile_;
	uint32_t currentTileSize_;
	int tagTableCount_;
	int stringCount_;

	friend class TileReader<TileCompiler>;
};
