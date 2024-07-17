// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once
// #include <unordered_map>
#include <common/alloc/Arena.h>
#include <common/util/log.h>
#include "TileReader.h"
#include "TFeature.h"
#include "TRelationTable.h"
#include "TString.h"
#include "TTagTable.h"
#include "geom/Tile.h"

// TODO: Call this TileKit

class TTile : public TileReader<TTile>
{
public:
	TTile(Tile tile);
	void readTile(pointer pTile);

	TReferencedElement* getElement(const void* p) const
	{
		return elementsByLocation_.lookup(currentLocation(pointer(p)));
	}

	TTagTable* getTags(const void* p) const
	{
		p = reinterpret_cast<const void*>(reinterpret_cast<uintptr_t>(p) & ~1);
		return reinterpret_cast<TTagTable*>(elementsByLocation_.lookup(
			currentLocation(pointer(p))));
	}

	TString* getString(const void* p) const
	{
		return reinterpret_cast<TString*>(elementsByLocation_.lookup(
			currentLocation(pointer(p))));
	}

	TRelationTable* getRelationTable(const void* p) const
	{
		return reinterpret_cast<TRelationTable*>(elementsByLocation_.lookup(
			currentLocation(pointer(p))));
	}

	Arena& arena() { return arena_; }
	Box bounds() const { return tile_.bounds(); }
	uint32_t featureCount() const { return featureCount_; }
	const FeatureTable& features() const { return featuresById_; }
	const ElementDeduplicator<TString> strings() const { return strings_; }
	const ElementDeduplicator<TTagTable> tagTables() const { return tagTables_; }
	const ElementDeduplicator<TRelationTable> relationTables() const { return relationTables_; }

	FeatureTable::Iterator iterFeatures() const
	{
		return featuresById_.iter();
	}

	uint8_t* newTileData() const { return pNewTile_;	}
	uint8_t* write(Layout& layout);


private:
	void readNode(NodeRef node);
	void readWay(WayRef way);
	void readRelation(RelationRef relation);
	TString* readString(pointer p);
	TTagTable* readTagTable(pointer pTagged);
	TTagTable* readTagTable(FeatureRef feature)
	{
		TTagTable* tags = readTagTable(feature.tags().taggedPtr());
		tags->addUser();
		return tags;
	}
	TRelationTable* readRelationTable(pointer p);

	int32_t currentLocation(pointer p) const
	{
		return static_cast<int32_t>(pCurrentTile_ - p.asBytePointer());
		// We use negative values to indicate old location
		// TODO: Change this approach
	}

	/**
	 * 
	 */
	int32_t newRelativePointer(pointer p, int oldRel)
	{
		int pointerLoc = static_cast<int32_t>(pNewTile_ - p.asBytePointer());
		int oldLoc = pointerLoc + oldRel;
		// We use negative values to indicate old location
		TElement* elem = elementsByLocation_.lookup(oldLoc);
		assert(elem);
		return elem->location() - pointerLoc;
	}

	/**
	 * Asserts that the given pointer references a valid address in the
	 * current tile data (i.e. the existing tile that is being read).
	 */
	void assertValidCurrentPointer(const void* p)
	{
#ifdef _DEBUG
		if (p < pCurrentTile_ || p > pCurrentTile_ + currentTileSize_)
		{
			LOG("While reading % s in % s:", currentLoadingFeature_.toString().c_str(), tile_.toString().c_str());
			LOG("  Pointer:    %016llX", reinterpret_cast<uintptr_t>(p));
			LOG("  Tile start: %016llX", reinterpret_cast<uintptr_t>(pCurrentTile_));
			LOG("  Tile end:   %016llX", reinterpret_cast<uintptr_t>(pCurrentTile_ + currentTileSize_));
		}
#endif
		assert(p >= pCurrentTile_ && p <= pCurrentTile_ + currentTileSize_);
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
	uint8_t* pNewTile_;
	uint32_t currentTileSize_;
	uint32_t featureCount_;
	Tile tile_;
#ifdef _DEBUG
	FeatureRef currentLoadingFeature_;
#endif
	friend class TileReader<TTile>;
};
