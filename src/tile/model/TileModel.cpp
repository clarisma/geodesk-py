// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#include "TileModel.h"
#include <algorithm>
#include "TFeature.h"
#include "Layout.h"
#include <common/util/log.h>
#include <common/util/varint.h>

TileModel::TileModel(Tile tile) :
	arena_(1024 * 1024, Arena::GrowthPolicy::GROW_50_PERCENT),
	featureCount_(0),
	pCurrentTile_(nullptr),
	pNewTile_(nullptr),
	currentTileSize_(0),
	tile_(tile)
{
}

void TileModel::initTables(size_t tileSize)
{
	size_t minTableSize = 1;
	size_t tableSize = std::max(tileSize / 64 * 7, minTableSize);
	elementsByHandle_.init(arena_.allocArray<TReferencedElement*>(tableSize), tableSize);

	tableSize = std::max(tileSize / 512 * 37, minTableSize);
	featuresById_.init(arena_.allocArray<TFeature*>(tableSize), tableSize);

	tableSize = std::max(tileSize / 200, minTableSize);
	strings_.init(arena_.allocArray<TString*>(tableSize), tableSize);

	tableSize = std::max(tileSize / 90, minTableSize);
	tagTables_.init(arena_.allocArray<TTagTable*>(tableSize), tableSize);

	tableSize = std::max(tileSize / 3000, minTableSize);
	relationTables_.init(arena_.allocArray<TRelationTable*>(tableSize), tableSize);
}


TFeature* TileModel::getFeature(TypedFeatureId typedId) const
{
	return featuresById_.lookup(typedId.asIdBits());
}

/*
template <typename T>
T* TileKit::createSharedElement(const uint8_t* data, uint32_t unencodedSize)
{
	uint8_t* bytes = arena_.alloc(sizeof(TSharedElement) + unencodedSize, alignof(TSharedElement));
	T* element = reinterpret_cast<T*>(bytes);
	new(element) T(bytes, unencodedSize);
	memcpy(bytes + sizeof(TSharedElement), data, unencodedSize);
	return element;
}
*/

/*
TString* TileKit::addString(const uint8_t* p, uint32_t size)
{
	TString* str = createSharedElement<TString>(p, size);
	strings_.insertUnique(str);
}
*/

TString* TileModel::addUniqueString(TElement::Handle handle, const uint8_t* p, uint32_t size)
{
	TString* str = arena_.create<TString>(handle, p, size);
	strings_.insertUnique(str);
	elementsByHandle_.insert(str);
	return str;
}

TString* TileModel::addUniqueString(DataPtr p)
{
	return addUniqueString(existingHandle(p), p, TString::getStringSize(p));
}


/*
TString* TileKit::addString(DataPtr p)
{
	TString* str = arena_.alloc<TString>();
	new(str) TString(currentLocation(p), p);
	elementsByLocation_.insert(str);
	strings_.insertUnique(str);
}
*/


TTagTable* TileModel::addTagTable(TElement::Handle handle,
	const uint8_t* data, uint32_t size, uint32_t hash, uint32_t anchor)
{
	TTagTable* tags = arena_.create<TTagTable>(handle, data, size, hash, anchor);
	elementsByHandle_.insert(tags);
	tagTables_.insertUnique(tags);
	return tags;
}

TTagTable* TileModel::beginTagTable(uint32_t size, uint32_t anchor)
{
	uint8_t* bytes = arena_.alloc(sizeof(TTagTable) + size, alignof(TTagTable));
	return new(bytes) TTagTable (0, bytes + sizeof(TTagTable),
		size, 0, anchor);
}

TTagTable* TileModel::completeTagTable(TTagTable* tags, uint32_t hash)
{
	// TODO
	return tags;
}

TRelationTable* TileModel::addRelationTable(TElement::Handle handle, const uint8_t* data,
	uint32_t size, uint32_t hash)
{
	TRelationTable* rels = arena_.create<TRelationTable>(handle, data, size, hash);
	elementsByHandle_.insert(rels);
	relationTables_.insertUnique(rels);
	return rels;
}


TNode* TileModel::addNode(NodePtr node)
{
	TNode* tnode = arena_.create<TNode>(existingHandle(node), node);
	addFeatureToIndex(tnode);
	return tnode;
}


TWay* TileModel::addWay(WayPtr way, DataPtr pBodyStart, uint32_t bodySize, uint32_t bodyAnchor)
{
	TWay* tway = arena_.create<TWay>(
		existingHandle(way), way, pBodyStart, bodySize, bodyAnchor);
	addFeatureToIndex(tway);
	return tway;
}


TRelation* TileModel::addRelation(RelationPtr rel, DataPtr pBodyStart, uint32_t bodySize)
{
	TRelation* trel = arena_.create<TRelation>(
		existingHandle(rel), rel, pBodyStart, bodySize);
	addFeatureToIndex(trel);
	return trel;
}
	

uint8_t* TileModel::write(Layout& layout)
{
	LOG("Old size: %d | New size: %d", currentTileSize_, layout.size());

	pNewTile_ = new uint8_t[layout.size()];
	TElement* elem = layout.first();
	do
	{
		switch (elem->type())
		{
		case TElement::Type::FEATURE:
			reinterpret_cast<TFeature*>(elem)->write(*this);
			break;
		case TElement::Type::WAY_BODY:
			reinterpret_cast<TWayBody*>(elem)->write(*this);
			break;
		case TElement::Type::RELATION_BODY:
			reinterpret_cast<TRelationBody*>(elem)->write(*this);
			break;
		case TElement::Type::STRING:
			reinterpret_cast<TString*>(elem)->write(newTileData() + elem->location());
			break;
		case TElement::Type::TAGS:
			reinterpret_cast<TTagTable*>(elem)->write(*this);
			break;
		case TElement::Type::RELTABLE:
			reinterpret_cast<TRelationTable*>(elem)->write(*this);
			break;
		case TElement::Type::INDEX:
			reinterpret_cast<TIndex*>(elem)->write(*this);
			break;
		case TElement::Type::TRUNK:
			reinterpret_cast<TIndexTrunk*>(elem)->write(*this);
			break;

		}
		elem = elem->next();
	}
	while (elem);
	return pNewTile_;
}
