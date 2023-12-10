#include "TTile.h"
#include <algorithm>
#include "TFeature.h"
#include "Layout.h"
#include <common/util/log.h>
#include <common/util/varint.h>

TTile::TTile(Tile tile) :
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

void TTile::initTables(size_t tileSize)
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

void TTile::readTile(pointer pTile)
{
	uint32_t tileSize = (pTile.getInt() & 0x3fff'ffff) + 4;
		// Need to add header size (4 bytes) to payload size

	pCurrentTile_ = pTile;
	currentTileSize_ = tileSize;
	initTables(tileSize);
	readTileFeatures(pTile);

	// Now we have all features, tag-tables and strings in the old tile,
	// indexed by location (also indexed by ID for features)
}


void TTile::readNode(NodeRef node)
{
	assertValidCurrentPointer(node.ptr());
	readTagTable(node);
	if (node.isRelationMember()) readRelationTable(node.bodyptr());
	TNode* tnode = arena_.alloc<TNode>();
	new(tnode) TNode(currentLocation(node.ptr()), node);
	addFeatureToIndex(tnode);
}

void TTile::readWay(WayRef way)
{
#ifdef _DEBUG
	currentLoadingFeature_ = way;
#endif
	// LOG("Reading %s in %s...", way.toString().c_str(), tile_.toString().c_str());
	assertValidCurrentPointer(way.ptr());
	readTagTable(way);
	TWay* tway = arena_.alloc<TWay>();
	pointer pBody = way.bodyptr();
	uint32_t relTablePtrSize = way.flags() & 4;
	uint32_t anchor;
	if (way.flags() & FeatureFlags::WAYNODE)
	{
		pointer pNode(pBody);
		pNode -= relTablePtrSize;		// skip pointer to reltable (4 bytes)
		for(;;)
		{
			pNode -= 4;
			int32_t wayNode = pNode.getUnalignedInt();
			if ((wayNode & (MemberFlags::FOREIGN | MemberFlags::DIFFERENT_TILE)) ==
				(MemberFlags::FOREIGN | MemberFlags::DIFFERENT_TILE))
			{
				// foreign node in different tile
				pNode -= 2;
				pNode -= (pNode.getShort() & 1) << 1;   
					// move backward by 2 extra bytes if the tip-delta is wide
					// (denoted by Bit 0)
			}
			if (wayNode & MemberFlags::LAST) break;
		}
		anchor = pBody - pNode;
	}
	else
	{
		anchor = relTablePtrSize;
	}

	const uint8_t* p = pBody;
	int nodeCount = readVarint32(p);
	skipVarints(p, nodeCount * 2);		// (coordinate pairs)
	uint32_t size = pointer(p) - pBody + anchor;
	if (relTablePtrSize)
	{
		readRelationTable(pBody.followUnaligned(-4));
	}

	new(tway) TWay(currentLocation(way.ptr()), way, pBody - anchor, size, anchor);
	addFeatureToIndex(tway);
}

void TTile::readRelation(RelationRef relation)
{
	// LOG("Reading relation/%ld", relation.id());
	if (relation.id() == 184508)
	{
		LOG("Reading %s", relation.toString().c_str());
	}
	assertValidCurrentPointer(relation.ptr());
	readTagTable(relation);
	TRelation* trel = arena_.alloc<TRelation>();
	pointer pBody = relation.bodyptr();
	
	pointer p(pBody);
	for (;;)
	{
		int32_t member = p.getUnalignedInt();
		p += 4;
		if ((member & (MemberFlags::FOREIGN | MemberFlags::DIFFERENT_TILE)) ==
			(MemberFlags::FOREIGN | MemberFlags::DIFFERENT_TILE))
		{
			// foreign member in different tile
			p += 2 + ((p.getShort() & 1) << 1);
			// move forward by 2 extra bytes if the tip-delta is wide
			// (denoted by Bit 0)
		}
		if (member & MemberFlags::DIFFERENT_ROLE)
		{
			int32_t rawRole = p.getUnsignedShort();
			p += 2;
			if ((rawRole & 1) == 0) // local-string role?
			{
				rawRole |= static_cast<int32_t>(p.getShort()) << 16;
				readString(p + (rawRole >> 1) - 2);   // signed
				p += 2;
			}
		}
		if (member & MemberFlags::LAST) break;
	}

	uint32_t anchor = 0;
	uint32_t size = p - pBody;
	if (relation.flags() & FeatureFlags::RELATION_MEMBER)
	{
		readRelationTable(pBody.follow(-4));		// TODO: This is an unaligned read!
		size += 4;
		anchor = 4;
	}

	new(trel) TRelation(currentLocation(relation.ptr()), relation, pBody - anchor, size);
	addFeatureToIndex(trel);
}

TString* TTile::readString(pointer p)
{
	assertValidCurrentPointer(p);
	int32_t currentLoc = currentLocation(p);
	TString* str = reinterpret_cast<TString*>(elementsByLocation_.lookup(currentLoc));
	if (!str)
	{
		str = arena_.alloc<TString>();
		new(str) TString(currentLoc, p);
		elementsByLocation_.insert(str);
		strings_.insertUnique(str);
	}
	str->addUser();
	return str;
}


TTagTable* TTile::readTagTable(pointer pTagged)
{
	pointer pTags = pointer::ofTagged(pTagged, ~1);
	assertValidCurrentPointer(pTags);
	int32_t currentLoc = currentLocation(pTags);
	int hasLocalTags = reinterpret_cast<uintptr_t>(pTagged.asBytePointer()) & 1;
	assert((currentLoc & 1) == 0);
	TTagTable* tags = reinterpret_cast<TTagTable*>(elementsByLocation_.lookup(currentLoc));
	if (tags) return tags;

	int anchor;
	if (hasLocalTags)
	{
		pointer p = pTags;
		pointer origin = pointer::ofTagged(p, 0xffff'ffff'ffff'fffcULL);
		for (;;)
		{
			p -= 4;
			int32_t key = p.getUnalignedInt();
			int flags = key & 7;
			pointer pKeyString = origin + ((key ^ flags) >> 1);
			TString* keyString = readString(pKeyString);
			// Force string to be 4-byte aligned
			keyString->setAlignment(TElement::Alignment::DWORD);
			p -= 2 + (flags & 2);
			if ((flags & 3) == 3) // wide-string value?
			{
				readString(p.follow());   // TODO: This is an unaligned read!
			}
			if (flags & 4) break;  // last-tag?
		}
		anchor = pTags - p;
	}
	else
	{
		anchor = 0;
	}
	
	uint32_t size;
	pointer p = pTags;
	if (p.getUnalignedUnsignedInt() == TagsRef::EMPTY_TABLE_MARKER)
	{
		// TODO: This will change, no more need for special check
		size = 4;
	}
	else
	{
		for (;;)
		{
			uint16_t key = p.getUnsignedShort();
			if ((key & 3) == 3)	// wide-string value?
			{
				readString(p.follow(2));
			}
			p += (key & 2) + 4;
			if (key & 0x8000) break;	// last global key
		}
		size = p - pTags + anchor;
	}

	assert((currentLoc & 1) == 0);
	tags = arena_.alloc<TTagTable>();
	new(tags) TTagTable(currentLoc, pTags-anchor, size, anchor);
	elementsByLocation_.insert(tags);
	tagTables_.insertUnique(tags);
	return tags;
}


TRelationTable* TTile::readRelationTable(pointer pTable)
{
	int32_t currentLoc = currentLocation(pTable);
	TRelationTable* rels = getRelationTable(pTable);
	if (!rels)
	{
		pointer p = pTable;
		for (;;)
		{
			int32_t rel = p.getUnalignedInt();
			p += 4;
			if ((rel & (MemberFlags::FOREIGN | MemberFlags::DIFFERENT_TILE)) ==
				(MemberFlags::FOREIGN | MemberFlags::DIFFERENT_TILE))
			{
				// foreign relation in different tile
				p += 2;
				p += (p.getShort() & 1) << 1;
				// move forward by 2 extra bytes if the tip-delta is wide
				// (denoted by Bit 0)
			}
			if (rel & MemberFlags::LAST) break;
		}
		uint32_t size = p - pTable;
		rels = arena_.alloc<TRelationTable>();
		new(rels) TRelationTable(currentLocation(pTable), pTable, size);
		elementsByLocation_.insert(rels);
		relationTables_.insertUnique(rels);
	}
	rels->addUser();
	return rels;
}



uint8_t* TTile::write(Layout& layout)
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
