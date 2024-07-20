// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#include "TileReader.h"
#include <common/util/log.h>
#include <common/util/varint.h>

void TileReader::readTile(const DataPtr pTile)
{
	uint32_t tileSize = (pTile.getInt() & 0x3fff'ffff) + 4;
	// Need to add header size (4 bytes) to payload size
	// TODO: will change

	tile_.setSource(pTile, tileSize);
	tile_.initTables(tileSize);
	readTileFeatures(pTile);

	// Now we have all features, tag-tables and strings in the old tile,
	// indexed by location (also indexed by ID for features)
}


void TileReader::readNode(NodePtr node)
{
	readTagTable(node);
	if (node.isRelationMember()) readRelationTable(node.bodyptr());
	tile_.addNode(node);
}

void TileReader::readWay(WayPtr way)
{
	// LOG("Reading %s in %s...", way.toString().c_str(), tile_.toString().c_str());
	readTagTable(way);
	DataPtr pBody = way.bodyptr();
	uint32_t relTablePtrSize = way.flags() & 4;
	uint32_t anchor;
	if (way.flags() & FeatureFlags::WAYNODE)
	{
		DataPtr pNode(pBody);
		pNode -= relTablePtrSize;		// skip pointer to reltable (4 bytes)
		for (;;)
		{
			pNode -= 4;
			int32_t wayNode = pNode.getIntUnaligned();
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
		anchor = static_cast<uint32_t>(pBody - pNode);
	}
	else
	{
		anchor = relTablePtrSize;
	}

	const uint8_t* p = pBody;
	int nodeCount = readVarint32(p);
	skipVarints(p, nodeCount * 2);		// (coordinate pairs)
	uint32_t size = p - pBody + anchor;
	if (relTablePtrSize)
	{
		readRelationTable((pBody-4).followUnaligned());
	}
	tile_.addWay(way, pBody - anchor, size, anchor);
}

void TileReader::readRelation(RelationPtr relation)
{
	// LOG("Reading relation/%ld", relation.id());
	DataPtr pBody = relation.bodyptr();
	DataPtr p = pBody;
	for (;;)
	{
		int32_t member = p.getIntUnaligned();
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
	DataPtr bodyData = pBody;
	if (relation.flags() & FeatureFlags::RELATION_MEMBER)
	{
		readRelationTable((pBody - 4).followUnaligned());	
		size += 4;
		anchor = 4;
		bodyData -= 4;
	}
	tile_.addRelation(relation, bodyData, size);
}

TString* TileReader::readString(DataPtr p)
{
	TString* str = tile_.getString(tile_.existingHandle(p));
	if (!str) tile_.addUniqueString(p);
	str->addUser();
	return str;
}

// Hash is calculated as follows: 
// local tags (traversal order), then global tags (traversal order),

// TODO: addUser to tags!!!

TTagTable* TileReader::readTagTable(TaggedPtr<const uint8_t, 1> pTagged)
{
	DataPtr pTags = pTagged.ptr();
	int hasLocalTags = pTagged.flags();
	TElement::Handle handle = tile_.existingHandle(pTags);
	TTagTable* tags = tile_.getTags(handle);
	if (tags) return tags;

	TTagTable::Hasher hasher;

	int anchor;
	if (hasLocalTags)
	{
		DataPtr p = pTags;
		DataPtr origin = pointer::ofTagged(p, 0xffff'ffff'ffff'fffcULL);
		for (;;)
		{
			p -= 4;
			int32_t key = p.getIntUnaligned();
			int flags = key & 7;
			DataPtr pKeyString = origin + ((key ^ flags) >> 1);
			TString* keyString = readString(pKeyString);
			// Force string to be 4-byte aligned
			keyString->setAlignment(TElement::Alignment::DWORD);
			hasher.addKey(keyString);
			p -= 2 + (flags & 2);
			if (flags & 2) // wide value?
			{
				if (flags & 1) // wide-string value?
				{
					hasher.addValue(readString(p.followUnaligned()));
				}
				else
				{
					hasher.addValue(p.getUnsignedIntUnaligned());
				}
			}
			else
			{
				hasher.addValue(p.getUnsignedShort());
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
	DataPtr p = pTags;
	if (p.getUnsignedIntUnaligned() == TagsRef::EMPTY_TABLE_MARKER)
	{
		// TODO: This will change, no more need for special check
		size = 4;
	}
	else
	{
		for (;;)
		{
			uint16_t key = p.getUnsignedShort();
			hasher.addKey((key & 0x7fff) >> 2);
			p += 2;
			if (key & 2)	// wide value?
			{
				if (key & 1)	// wide-string value?
				{
					hasher.addValue(readString(p.followUnaligned()));
				}
				else
				{
					hasher.addValue(p.getUnsignedIntUnaligned());
				}
			}
			else
			{
				hasher.addValue(p.getUnsignedShort());
			}
			p += (key & 2) + 2;
			if (key & 0x8000) break;	// last global key
		}
		size = p - pTags + anchor;
	}

	return tile_.addTagTable(tile_.existingHandle(pTags),
		pTags - anchor, size, hasher.hash(), anchor);
}


TRelationTable* TileReader::readRelationTable(DataPtr pTable)
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

