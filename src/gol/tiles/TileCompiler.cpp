#include "TileCompiler.h"
#include "TFeature.h"
#include <common/util/varint.h>

TileCompiler::TileCompiler()
{

}

void TileCompiler::readTile(pointer pTile)
{
	uint32_t tileSize = pTile.getInt() & 0x3fff'ffff;

	size_t tableSize = static_cast<size_t>(tileSize) / 64 * 7;
	elementsByLocation_.init(arena_.allocArray<TIndexedElement*>(tableSize), tableSize);
	readTileFeatures(pTile);
}


void TileCompiler::readNode(NodeRef node)
{
	TNode* tnode = arena_.alloc<TNode>();
	new(tnode) TNode(currentLocation(node.ptr()), node);
	// TODO: add to feature index
}

void TileCompiler::readWay(WayRef way)
{
	TWay* tway = arena_.alloc<TWay>();
	pointer pBody = way.bodyptr();
	const uint8_t* p = pBody;
	int nodeCount = readVarint32(p);
	skipVarints(p, nodeCount * 2);		// (coordinate pairs)
	uint32_t size = p - pBody;
	uint32_t anchor;
	TElement::SizeAndAlignment sizeAndAlignment = TElement::unaligned(size);
	if (way.flags() & FeatureFlags::WAYNODE)
	{
		pointer pNode(pBody);
		pNode -= way.flags() & 4;	// skip pointer to reltable if Bit 2 set
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
					// move forward by 2 extra bytes if the tip-delta is wide
					// (denoted by Bit 0)
			}
			if (wayNode & MemberFlags::LAST) break;
		}
		anchor = pBody - pNode;
		size += anchor;
		sizeAndAlignment = TElement::aligned2(size);
	}
	else
	{
		anchor = 0;
	}
	new(tway) TWay(currentLocation(way.ptr()), way, pBody, sizeAndAlignment, anchor);
	// TODO: add to feature index
}

void TileCompiler::readRelation(RelationRef relation)
{

}

TString* TileCompiler::readString(pointer p)
{
	int32_t currentLoc = currentLocation(p);
	TString* str = reinterpret_cast<TString*>(elementsByLocation_.lookup(currentLoc));
	if (str) return str;
	str = arena_.alloc<TString>();
	new(str) TString(currentLoc, p);
	elementsByLocation_.insert(str);
	return str;
}


TTagTable* TileCompiler::readTagTable(pointer pTagged)
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
	return tags;
}
