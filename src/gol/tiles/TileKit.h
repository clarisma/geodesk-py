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


class TileKit
{
public:
	TileKit(Tile tile);

	void setSource(const uint8_t* pTile, uint32_t size)
	{
		pCurrentTile_ = pTile;
		currentTileSize_ = size;
	}

	void initTables(size_t tileSize);
	
	TString* addString(int32_t location, const uint8_t* p, uint32_t size);
	TString* addString(DataPtr p);
	void addNode(NodeRef node);
	void addWay(WayRef way, DataPtr pBodyStart, uint32_t bodySize, uint32_t bodyAnchor);
	
	TReferencedElement* getElement(const uint8_t* p) const
	{
		return elementsByLocation_.lookup(currentLocation(pointer(p)));
	}

	TTagTable* getTags(const uint8_t* p) const
	{
		p = reinterpret_cast<const void*>(reinterpret_cast<uintptr_t>(p) & ~1);
		TTagTable* tags = reinterpret_cast<TTagTable*>(elementsByLocation_.lookup(
			currentLocation(pointer(p))));
		assert(!tags || tags->type() == TElement::Type::TAGS);
		return tags;
	}

	TString* getString(const uint8_t* p) const
	{
		TString* s = reinterpret_cast<TString*>(elementsByLocation_.lookup(
			currentLocation(pointer(p))));
		assert(!s || s->type() == TElement::Type::STRING);
		return s;
	}

	TRelationTable* getRelationTable(const void* p) const
	{
		TRelationTable* rels = reinterpret_cast<TRelationTable*>(
			elementsByLocation_.lookup(currentLocation(pointer(p))));
		// assert(!rels || rels->type() == TElement::Type::RELTABLE);
		if (rels && rels->type() != TElement::Type::RELTABLE)
		{
			printf("  Requested location: %d\n", currentLocation(pointer(p)));
			printf("    Element location: %d\n", rels->oldLocation());
		}
		return rels;
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

	uint8_t* alloc(uint8_t size) {return arena_.alloc(size, 4); }

private:
	
	int32_t currentLocation(const uint8_t* p) const
	{
		return static_cast<int32_t>(pCurrentTile_ - p);
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
