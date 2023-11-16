#pragma once
// #include <unordered_map>
#include <common/alloc/Arena.h>
#include "TileReader.h"
#include "TFeature.h"
#include "TRelationTable.h"
#include "TString.h"
#include "TTagTable.h"
#include "geom/Tile.h"

class TTile : public TileReader<TTile>
{
public:
	TTile(Tile tile);
	void readTile(pointer pTile);

	TIndexedElement* getElement(const void* p) const
	{
		return elementsByLocation_.lookup(reinterpret_cast<const uint8_t*>(p) - pCurrentTile_);
	}

	Arena& arena() { return arena_; }
	Box bounds() const { return tile_.bounds(); }
	uint32_t featureCount() const { return featureCount_; }

	FeatureTable::Iterator iterFeatures() const
	{
		return featuresById_.iter();
	}

	TTagTable* getTags(const void* p) const;

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
		featureCount_++;
	}

	void initTables(size_t tileSize);

	Arena arena_;
	LookupByLocation elementsByLocation_;
	FeatureTable featuresById_;
	ElementDeduplicator<TString> strings_;
	ElementDeduplicator<TTagTable> tagTables_;
	ElementDeduplicator<TRelationTable> relationTables_;
	const uint8_t* pCurrentTile_;
	uint32_t currentTileSize_;
	uint32_t featureCount_;
	Tile tile_;

	friend class TileReader<TTile>;
};
