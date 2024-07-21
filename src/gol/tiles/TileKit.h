// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once
// #include <unordered_map>
#include <common/alloc/Arena.h>
#include <common/util/log.h>
#include "TFeature.h"
#include "TRelationTable.h"
#include "TString.h"
#include "TTagTable.h"
#include "geom/Tile.h"

class TileModel
{
public:
	TileModel(Tile tile);

	void setSource(const uint8_t* pTile, uint32_t size)
	{
		pCurrentTile_ = pTile;
		currentTileSize_ = size;
	}

	void initTables(size_t tileSize);
	
	TFeature* getFeature(TypedFeatureId typedId) const;

	TElement::Handle newHandle();
	TString* addString(const uint8_t* p, uint32_t size);
	TString* addUniqueString(TElement::Handle handle, const uint8_t* p, uint32_t size);
	TString* addUniqueString(DataPtr p);
	TTagTable* addTagTable(TElement::Handle handle, const uint8_t* data, 
		uint32_t size, uint32_t hash, uint32_t anchor);
	TRelationTable* addRelationTable(TElement::Handle handle, const uint8_t* data,
		uint32_t size, uint32_t hash);

	TTagTable* beginTagTable(uint32_t size, uint32_t anchor);
	TTagTable* completeTagTable(TTagTable* tags, uint32_t hash);

	TNode* addNode(NodePtr node);
	TWay* addWay(WayPtr way, DataPtr pBodyStart, uint32_t bodySize, uint32_t bodyAnchor);
	TRelation* addRelation(RelationPtr rel, DataPtr pBodyStart, uint32_t bodySize);
	
	TReferencedElement* getElement(TElement::Handle handle) const
	{
		return elementsByHandle_.lookup(handle);
	}

	TTagTable* getTags(TElement::Handle handle) const
	{
		return TElement::cast<TTagTable>(getElement(handle));
	}

	TString* getString(TElement::Handle handle) const
	{
		return TElement::cast<TString>(getElement(handle));
	}

	TRelationTable* getRelationTable(TElement::Handle handle) const
	{
		return TElement::cast<TRelationTable>(getElement(handle));
	}


	/*
	TString* getString(const uint8_t* s, uint32_t size) const
	{
		TString* s = strings_.reinterpret_cast<TString*>(elementsByLocation_.lookup(
			currentLocation(pointer(p))));
		assert(!s || s->type() == TElement::Type::STRING);
		return s;
	}
	*/


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

	TElement::Handle existingHandle(DataPtr p) const
	{
		TElement::Handle handle = static_cast<TElement::Handle>(p - pCurrentTile_);
		assert(handle > 0 && handle < currentTileSize_);
		return handle;
	}

private:
	void addFeatureToIndex(TFeature* feature)
	{
		elementsByHandle_.insert(feature);
		featuresById_.insert(feature);
		featureCount_++;
	}

	Arena arena_;
	LookupByHandle elementsByHandle_;
	FeatureTable featuresById_;
	ElementDeduplicator<TString> strings_;
	ElementDeduplicator<TTagTable> tagTables_;
	ElementDeduplicator<TRelationTable> relationTables_;
	const uint8_t* pCurrentTile_;
	uint8_t* pNewTile_;
	uint32_t currentTileSize_;
	uint32_t featureCount_;
	Tile tile_;
};
