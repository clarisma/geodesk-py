#include "TTile.h"
#include <algorithm>
#include "TFeature.h"
#include <common/util/log.h>
#include <common/util/varint.h>

TTile::TTile(Tile tile) :
	arena_(1024 * 1024, Arena::GrowthPolicy::GROW_50_PERCENT),
	featureCount_(0),
	tile_(tile)
{
}

void TTile::initTables(size_t tileSize)
{
	size_t minTableSize = 1;
	size_t tableSize = std::max(tileSize / 64 * 7, minTableSize);
	elementsByLocation_.init(arena_.allocArray<TIndexedElement*>(tableSize), tableSize);

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
	uint32_t tileSize = pTile.getInt() & 0x3fff'ffff;

	initTables(tileSize);
	readTileFeatures(pTile);

	// Now we have all features, tag-tables and strings in the old tile,
	// indexed by location (also indexed by ID for features)
}


void TTile::readNode(NodeRef node)
{
	TNode* tnode = arena_.alloc<TNode>();
	new(tnode) TNode(currentLocation(node.ptr()), node);
	addFeatureToIndex(tnode);
}

void TTile::readWay(WayRef way)
{
	TWay* tway = arena_.alloc<TWay>();
	pointer pBody = way.bodyptr();
	uint32_t size = way.flags() & 4;
	uint32_t anchor;
	if (way.flags() & FeatureFlags::WAYNODE)
	{
		pointer pNode(pBody);
		pNode -= size;		// skip pointer to reltable (4 bytes)
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
		size += anchor;
	}
	else
	{
		anchor = 0;
	}

	const uint8_t* p = pBody;
	int nodeCount = readVarint32(p);
	skipVarints(p, nodeCount * 2);		// (coordinate pairs)
	uint32_t relTablePtrSize = way.flags() & 4;
	size += pointer(p) - pBody;

	new(tway) TWay(currentLocation(way.ptr()), way, pBody - anchor, size, anchor);
	addFeatureToIndex(tway);
}

void TTile::readRelation(RelationRef relation)
{
	TRelation* trel = arena_.alloc<TRelation>();
	pointer pBody = relation.bodyptr();
	uint32_t anchor = relation.flags() & 4;;
	uint32_t size = anchor;

	// LOG("Reading relation/%ld", relation.id());

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
	size += p - pBody;
	new(trel) TRelation(currentLocation(relation.ptr()), relation, pBody - anchor, size);
	addFeatureToIndex(trel);
}

TString* TTile::readString(pointer p)
{
	int32_t currentLoc = currentLocation(p);
	TString* str = reinterpret_cast<TString*>(elementsByLocation_.lookup(currentLoc));
	if (str) return str;
	str = arena_.alloc<TString>();
	new(str) TString(currentLoc, p);
	elementsByLocation_.insert(str);
	strings_.insertUnique(str);
	return str;
}


TTagTable* TTile::readTagTable(pointer pTagged)
{
	int32_t currentLoc = currentLocation(pTagged);
	int hasLocalTags = currentLoc & 1;
	currentLoc ^= hasLocalTags;
	TTagTable* tags = reinterpret_cast<TTagTable*>(elementsByLocation_.lookup(currentLoc));
	if (tags) return tags;

	int anchor;
	pointer pTags;
	if (hasLocalTags)
	{
		pTags = pTagged - 1;
		pointer p = pTags;
		pointer origin = pointer::ofTagged(p, 0xffff'ffff'ffff'fffcULL);
		for (;;)
		{
			p -= 4;
			int32_t key = p.getUnalignedInt();
			int flags = key & 7;
			pointer pKeyString = origin + ((key ^ flags) >> 1);
			TString* keyString = readString(pKeyString);
			// TODO: force string to be 4-byte aligned
			p -= 2 + (flags & 2);
			if ((flags & 3) == 3) // wide-string value?
			{
				readString(p.follow());
			}
			if (flags & 1) break;  // last-tag flag?
		}
		anchor = pTags - p;
	}
	else
	{
		anchor = 0;
		pTags = pTagged;
	}
	
	pointer p = pTags;
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
	uint32_t size = p = pTags + anchor;

	tags = arena_.alloc<TTagTable>();
	new(tags) TTagTable(currentLoc, pTags-anchor, size, anchor);
	elementsByLocation_.insert(tags);
	tagTables_.insertUnique(tags);
	return tags;
}


TRelationTable* TTile::readRelationTable(pointer pTable)
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
	TRelationTable* rels = arena_.alloc<TRelationTable>();
	new(rels) TRelationTable(currentLocation(pTable), pTable, size);
	elementsByLocation_.insert(rels);
	relationTables_.insertUnique(rels);
	return rels;
}


TTagTable* TTile::getTags(const void* p) const
{
	return reinterpret_cast<TTagTable*>(elementsByLocation_.lookup(
		reinterpret_cast<const uint8_t*>(p) - pCurrentTile_));
}
