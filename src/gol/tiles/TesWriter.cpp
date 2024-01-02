// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#include "TesWriter.h"
#include "TFeature.h"

TesWriter::TesWriter(TTile& tile, Buffer* out) :
	tile_(tile),
	out_(out),
	prevXY_(0,0),
	prevId_(0)
{
}


void TesWriter::write()
{
	writeStrings();
	writeTagTables();
	writeRelationTables();
	writeFeatures();
	out_.flush();
}


void TesWriter::writeStrings()
{
	TString** strings = sortedItems<TString>(tile_.strings());
	size_t count = tile_.strings().count();
	out_.writeVarint(count);
	TString** pEnd = strings + count;
	TString** p = strings;
	while (p < pEnd)
	{
		out_.writeBytes((*p)->data(), (*p)->size());
		p++;
	}
}


void TesWriter::writeTagTables()
{
	TTagTable** tagTables = sortedItems<TTagTable>(tile_.tagTables());
	size_t count = tile_.tagTables().count();
	out_.writeVarint(count);
	TTagTable** pEnd = tagTables + count;
	TTagTable** p = tagTables;
	while (p < pEnd)
	{
		writeTagTable(*p++);
	}
}


void TesWriter::writeRelationTables()
{
	TRelationTable** relationTables = sortedItems<TRelationTable>(tile_.relationTables());
	size_t count = tile_.relationTables().count();
	out_.writeVarint(count);
	TRelationTable** pEnd = relationTables + count;
	TRelationTable** p = relationTables;
	while (p < pEnd)
	{
		writeRelationTable(*p++);
	}
}


template <typename T>
T** TesWriter::sortedItems(const ElementDeduplicator<T>& items)
{
	size_t count = items.count();
	T** sorted = items.toArray(tile_.arena());

	std::sort(sorted, sorted+count, [](const T* a, const T* b) 
		{
			// Sort in descending order based on the result of users()
			return a->users() > b->users();
		});

	// TODO: group and sort by contents

	for (int i = 0; i < count; i++)
	{
		sorted[i]->setLocation(i + 1);
	}
	return sorted;
}


void TesWriter::writeTagValue(pointer p, int valueFlags)
{
	assert(valueFlags >= 0 && valueFlags <= 3);
	uint32_t value;
	if ((valueFlags & 2) == 0)		  // narrow value
	{
		value = p.getUnsignedShort();
	}
	else                              // wide value
	{
		value = p.getUnalignedUnsignedInt();
		if (valueFlags == 3)
		{
			TString* valueStr = tile_.getString(p + static_cast<int32_t>(value));
			assert(valueStr);
			value = valueStr->location();
		}
	}
	out_.writeVarint(value);
}

void TesWriter::writeTagTable(const TTagTable* tags)
{
	pointer pTable = tags->data() + tags->anchor();
	int globalTagCount = 0;
	int localTagCount = 0;
	pointer p = pTable;
	if (p.getUnalignedUnsignedInt() != TagsRef::EMPTY_TABLE_MARKER)
	{
		for(;;)
		{
			globalTagCount++;
			int key = p.getUnsignedShort();
			p += 2 + (key & 2);
			if (key & 0x8000) break;
		}
	}
	if (tags->hasLocalTags())
	{
		pointer origin = pointer::ofTagged(pTable, 0xffff'ffff'ffff'fffcULL);
		p = pTable;
		for (;;)
		{
			localTagCount++;
			p -= 4;
			int key = p.getUnsignedShort();
			p -= 2 + (key & 2);
			if (key & 4) break;
		}
		out_.writeVarint((globalTagCount << 1) | 1);
		out_.writeVarint(localTagCount);
		p = pTable;
		for (;;)
		{
			p -= 4;
			int32_t key = p.getUnalignedInt();
			int flags = key & 7;
			pointer pKeyString = origin + ((key ^ flags) >> 1);
			TString* keyStr = tile_.getString(pKeyString);
			int valueType = flags & 3;
			out_.writeVarint((keyStr->location() << 2) | valueType);
			p -= 2 + (valueType & 2);
			writeTagValue(p, valueType);
			if (flags & 4) break;  // last-tag?
		}
	}
	p = pTable;
	if (globalTagCount)		// have to check in case of empty table
	{
		for (;;)
		{
			uint16_t key = p.getUnsignedShort();
			out_.writeVarint(key & 0x7fff);
			int valueType = key & 3;
			p += 2;
			writeTagValue(p, valueType);
			p += 2 + (key & 2);
			if (key & 0x8000) break;
		}
	}
}


void TesWriter::writeStub(const TFeature* feature, int flagBitCount, int flags)
{
	int64_t id = feature->id();
	out_.writeVarint(((id - prevId_) << flagBitCount) | flags);
	prevId_ = id;
	out_.writeVarint(feature->tags(tile_)->location());
}


void TesWriter::writeNode(const TNode* node)
{
	NodeRef nodeRef(node->feature());
	bool isRelationMember = nodeRef.isRelationMember();
	writeStub(node, 3, TAGS_CHANGED | GEOMETRY_CHANGED | (isRelationMember ? RELATIONS_CHANGED : 0));

	if (isRelationMember)
	{
		TRelationTable* relTable = tile_.getRelationTable(nodeRef.bodyptr());
		out_.writeVarint(relTable->location());
	}
	Coordinate xy = nodeRef.xy();
	out_.writeSignedVarint(static_cast<int64_t>(xy.x) - prevXY_.x);
	out_.writeSignedVarint(static_cast<int64_t>(xy.y) - prevXY_.y);
	prevXY_ = xy;
}


void TesWriter::writeWay(const TWay* way)
{
	WayRef wayRef(way->feature());
	bool isRelationMember = wayRef.isRelationMember();
	bool hasFeatureNodes = (wayRef.flags() & FeatureFlags::WAYNODE);
	writeStub(way, 5, TAGS_CHANGED | GEOMETRY_CHANGED | 
		(isRelationMember ? RELATIONS_CHANGED : 0) |
		(wayRef.isArea() ? AREA_FEATURE : 0) |
		(hasFeatureNodes ? WAY_HAS_FEATURE_NODES : 0));
	
	pointer pBody = way->body().data();
	int anchor = way->body().anchor();
	if (isRelationMember)
	{
		TRelationTable* relTable = tile_.getRelationTable(pBody.followUnaligned(anchor-4));
		out_.writeVarint(relTable->location());
	}

	writeBounds(wayRef);
	out_.writeBytes(pBody.asBytePointer() + anchor, way->body().size() - anchor);

	if (hasFeatureNodes)
	{
		int skipReltablePointer = isRelationMember ? 4 : 0;
		out_.writeVarint(anchor - skipReltablePointer);
		pointer p = pBody + anchor - skipReltablePointer;
		for (;;)
		{
			p -= 4;
			int32_t node = p.getUnalignedInt();
			if (node & MemberFlags::FOREIGN)
			{
				uint32_t ref = static_cast<uint32_t>(node) >> 4;
				if (node & MemberFlags::DIFFERENT_TILE)
				{
					p -= 2;
					int32_t tipDelta = p.getShort();
					if (tipDelta & 1)
					{
						// wide TIP delta
						p -= 2;
						tipDelta = (tipDelta & 0xffff) |
							(static_cast<int32_t>(p.getShort()) << 16);
					}
					tipDelta >>= 1;     // signed
					out_.writeVarint((ref << 2) | 3);
					out_.writeSignedVarint(tipDelta);
				}
				else
				{
					out_.writeVarint((ref << 2) | 1);
				}
			}
			else
			{
				TNode* wayNode = reinterpret_cast<TNode*>(tile_.getElement(
					p + (static_cast<int32_t>((node >> 2) << 1))));
				// LOG("Way-node node/%lld", wayNode->id());
				out_.writeVarint(wayNode->location() << 1);
			}
			if (node & MemberFlags::LAST) break;
		}
	}
}

void TesWriter::writeRelation(const TRelation* relation)
{
	RelationRef relationRef(relation->feature());
	bool isRelationMember = relationRef.isRelationMember();
	writeStub(relation, 4, TAGS_CHANGED | GEOMETRY_CHANGED |
		(isRelationMember ? RELATIONS_CHANGED : 0) |
		(relationRef.isArea() ? AREA_FEATURE : 0));

	return;  // TODO

	LOG("Writing relation/%lld", relationRef.id());
	if (relationRef.id() == 181912)
	{
		LOG("debug");
	}


	const TRelationBody& body = relation->body();
	pointer pBody = body.data();
	if (isRelationMember)
	{
		TRelationTable* relTable = tile_.getRelationTable(pBody.followUnaligned());
		out_.writeVarint(relTable->location());
	}

	writeBounds(relationRef);

	int anchor = body.anchor();
	out_.writeVarint(body.size() - anchor);

	pointer p = pBody + anchor;
	for (;;)
	{
		int32_t member = p.getUnalignedInt();
		int rolechangedFlag = (member & MemberFlags::DIFFERENT_ROLE) ? 2 : 0;
		if (member & MemberFlags::FOREIGN)
		{
			uint32_t ref = static_cast<uint32_t>(member) >> 4;
			p += 4;
			if (member & MemberFlags::DIFFERENT_TILE)
			{
				int32_t tipDelta = p.getShort();
				p += 2;
				if (tipDelta & 1)
				{
					// wide TIP delta
					tipDelta = (tipDelta & 0xffff) |
						(static_cast<int32_t>(p.getShort()) << 16);
					p += 2;
				}
				tipDelta >>= 1;     // signed
				out_.writeVarint((ref << 3) | 5 | rolechangedFlag);
				out_.writeSignedVarint(tipDelta);
			}
			else
			{
				out_.writeVarint((ref << 3) | 1 | rolechangedFlag);
			}
		}
		else
		{
			pointer pMember = (p & 0xffff'ffff'ffff'fffcULL) + ((int32_t)(member & 0xffff'fff8) >> 1);
			TFeature* memberFeature = reinterpret_cast<TFeature*>(tile_.getElement(pMember));
			if (memberFeature == nullptr)
			{
				FeatureRef xxx(pMember);
				FORCE_LOG("Not found: %s", xxx.toString().c_str());
			}
			else
			{
				out_.writeVarint((memberFeature->location() << 2) | rolechangedFlag);
			}
			p += 4;
		}
		if (rolechangedFlag)
		{
			uint32_t roleValue;
			int rawRole = p.getUnsignedShort();
			if (rawRole & 1)
			{
				roleValue = rawRole;
				p += 2;
			}
			else
			{
				TString* roleStr = tile_.getString(p + (p.getUnalignedInt() >> 1));
				roleValue = roleStr->location() << 1;
				p += 4;
			}
			out_.writeVarint(roleValue);
		}
		if (member & MemberFlags::LAST) break;
	}
}




void TesWriter::writeBounds(FeatureRef feature)
{
	const Box& bounds = feature.bounds();
	out_.writeSignedVarint(static_cast<int64_t>(bounds.minX()) - prevXY_.x);
	out_.writeSignedVarint(static_cast<int64_t>(bounds.minY()) - prevXY_.y);
	out_.writeVarint(static_cast<uint64_t>(
		static_cast<int64_t>(bounds.maxX()) - bounds.minX()));
	out_.writeVarint(static_cast<uint64_t>(
		static_cast<int64_t>(bounds.maxY()) - bounds.minY()));
	prevXY_ = bounds.bottomLeft();
}


void TesWriter::writeFeatures()
{
	size_t count = tile_.featureCount();
	FeatureItem* features = tile_.arena().allocArray<FeatureItem>(count);
	FeatureItem* p = features;
	FeatureItem* pEnd = features + count;
	FeatureTable::Iterator iter(&tile_.features());
	while(p < pEnd)
	{
		TFeature* feature = iter.next();
		assert(feature);
		p->first = feature->id() | 
			(static_cast<uint64_t>(feature->feature().typeCode()) << 56);
		p->second = feature;
		p++;
	}

	std::sort(features, pEnd);
	p = features;
	while (p < pEnd)
	{
		if ((p->first >> 56) != 0) break;
		p++;
	}
	size_t nodeCount = p - features;
	while (p < pEnd)
	{
		if ((p->first >> 56) != 1) break;
		p++;
	}
	size_t wayCount = (p - features) - nodeCount;
	size_t relationCount = count - nodeCount - wayCount;

	p = features;
	pEnd = p + nodeCount;
	out_.writeVarint(nodeCount);
	while (p < pEnd)
	{
		writeNode(reinterpret_cast<const TNode*>(p->second));
		p++;
	}
	pEnd = p + wayCount;
	out_.writeVarint(wayCount);
	while (p < pEnd)
	{
		writeWay(reinterpret_cast<const TWay*>(p->second));
		p++;
	}
	pEnd = p + relationCount;
	out_.writeVarint(relationCount);
	while (p < pEnd)
	{
		writeRelation(reinterpret_cast<const TRelation*>(p->second));
		p++;
	}
}


void TesWriter::writeRelationTable(const TRelationTable* relTable)
{

}
