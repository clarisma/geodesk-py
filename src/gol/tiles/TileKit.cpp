// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#include "TileKit.h"
#include <algorithm>
#include "TFeature.h"
#include "Layout.h"
#include <common/util/log.h>
#include <common/util/varint.h>

TileKit::TileKit(Tile tile) :
	arena_(1024 * 1024, Arena::GrowthPolicy::GROW_50_PERCENT),
	featureCount_(0),
	pCurrentTile_(nullptr),
	pNewTile_(nullptr),
	currentTileSize_(0),
#ifdef _DEBUG
	currentLoadingFeature_(nullptr),
#endif
	tile_(tile)
{
}

void TileKit::initTables(size_t tileSize)
{
	size_t minTableSize = 1;
	size_t tableSize = std::max(tileSize / 64 * 7, minTableSize);
	elementsByLocation_.init(arena_.allocArray<TReferencedElement*>(tableSize), tableSize);

	tableSize = std::max(tileSize / 512 * 37, minTableSize);
	featuresById_.init(arena_.allocArray<TFeature*>(tableSize), tableSize);

	tableSize = std::max(tileSize / 200, minTableSize);
	strings_.init(arena_.allocArray<TString*>(tableSize), tableSize);

	tableSize = std::max(tileSize / 90, minTableSize);
	tagTables_.init(arena_.allocArray<TTagTable*>(tableSize), tableSize);

	tableSize = std::max(tileSize / 3000, minTableSize);
	relationTables_.init(arena_.allocArray<TRelationTable*>(tableSize), tableSize);
}

TString* TileKit::addString(int32_t location, const uint8_t* p, uint32_t size)
{
	TString* str = arena_.alloc<TString>();
	new(str) TString(location, p, size);
	elementsByLocation_.insert(str);
	strings_.insertUnique(str);
}


TString* TileKit::addString(DataPtr p)
{
	TString* str = arena_.alloc<TString>();
	new(str) TString(currentLocation(p), p);
	elementsByLocation_.insert(str);
	strings_.insertUnique(str);
}

void TileKit::addNode(NodeRef node)
{
	TNode* tnode = arena_.alloc<TNode>();
	new(tnode) TNode(currentLocation(node.ptr()), node);
	addFeatureToIndex(tnode);
}


void TileKit::addWay(WayRef way, DataPtr pBodyStart, uint32_t bodySize, uint32_t bodyAnchor)
{
	TWay* tway = arena_.alloc<TWay>();
	new(tway) TWay(currentLocation(way.ptr()), way, pBodyStart, bodySize, bodyAnchor);
	addFeatureToIndex(tway);
}
	

uint8_t* TileKit::write(Layout& layout)
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
