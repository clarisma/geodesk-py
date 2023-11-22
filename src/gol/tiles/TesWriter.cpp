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
	strings_ = sortedItems<TString>(tile_.strings());
	tagTables_ = sortedItems<TTagTable>(tile_.tagTables());
	relationTables_ = sortedItems<TRelationTable>(tile_.relationTables());
}


void TesWriter::writeStrings()
{
	size_t count = tile_.strings().count();
	out_.writeVarint(count);
	const TString** pEnd = strings_ + count;
	TString** p = strings_;
	while (p < pEnd)
	{
		out_.writeBytes((*p)->data(), (*p)->size());
	}
}

template <typename T>
T** TesWriter::sortedItems(const ElementDeduplicator<T>& items)
{
	size_t count = items.count();
	TSharedElement** sorted = items.toArray(tile_.arena());

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
	pointer p = pTable;
	if (globalTagCount)		// have to check in case of empty table
	{
		for (;;)
		{
			uint16_t key = p.getUnsignedShort();
			out_.writeVarint(key & 0x7fff);
			int valueType = key & 3;
			p += 2;
			writeTagValue(p, valueType);
			p += key & 2;
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
	if (isRelationMember)
	{
		TRelationTable* relTable = tile_.getRelationTable(pBody.followUnaligned(-4));
		out_.writeVarint(relTable->location());
	}

	writeBounds(wayRef);
	int anchor = way->body().anchor();
	out_.writeBytes(pBody.asBytePointer() + anchor, way->body().size() - anchor);

	if (hasFeatureNodes)
	{
		int skipReltablePointer = isRelationMember ? 4 : 0;
		out_.writeVarint(anchor - skipReltablePointer);
		pointer p = pBody - skipReltablePointer;
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
					p + (static_cast<int32_t>(node & 0xffff'fffc) >> 1)));
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

	const TRelationBody& body = relation->body();
	pointer pBody = body.data();
	if (isRelationMember)
	{
		TRelationTable* relTable = tile_.getRelationTable(pBody.followUnaligned(-4));
		out_.writeVarint(relTable->location());
	}

	writeBounds(relationRef);

	int anchor = body.anchor();
	out_.writeVarint(body.size() - anchor);

	pointer p = pBody + anchor;
	for (;;)
	{
		int32_t member = p.getUnalignedInt();
		if (member & MemberFlags::FOREIGN)
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
				p + (static_cast<int32_t>(node & 0xffff'fffc) >> 1)));
			out_.writeVarint(wayNode->location() << 1);
		}
		if (node & MemberFlags::LAST) break;
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
