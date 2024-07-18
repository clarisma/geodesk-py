// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#include "TesWriter.h"
#include "TesFlags.h"
#include "TFeature.h"

TesWriter::TesWriter(TTile& tile, Buffer* out) :
	tile_(tile),
	out_(out),
	prevXY_(0,0),
	nodeCount_(0),
	wayCount_(0)
{
}


void TesWriter::write()
{
	// TODO: Header
	writeFeatureIndex();
	writeStrings();
	writeTagTables();
	writeRelationTables();
	writeFeatures();
	out_.writeByte(0); // no removed features
	// TODO: ExportTable
	out_.flush();
}


void TesWriter::writeFeatureIndex()
{
	FeatureTable::Iterator iter = tile_.iterFeatures();
	while (iter.hasNext())
	{
		features_.push_back(SortedFeature(iter.next()));
	}
	std::sort(features_.begin(), features_.end());

	out_.writeVarint(features_.size());
	int prevType = 0;
	uint64_t prevId = 0;
	for (int i=0; i<features_.size(); i++)
	{
		const SortedFeature& feature = features_[i];
		int type = feature.typeCode();
		if (type != prevType)
		{
			if (type == 1)
			{
				nodeCount_ = i;
			}
			else
			{
				assert(type == 2);
				wayCount_ = i - nodeCount_;
			}
			out_.writeByte(0);
			prevType = type;
		}
		uint64_t id = feature.id();
		out_.writeVarint(((id - prevId) << 1) | 1);
			// Bit 0: changed_flag
		prevId = id;
		feature.feature()->setLocation(i + 1);
	}
}


void TesWriter::writeStrings()
{
	gatherSharedItems(tile_.strings(), 0, 127);
	out_.writeVarint(sharedElements_.size());
	for (const auto& e : sharedElements_)
	{
		TString* s = static_cast<TString*>(e);
		out_.writeBytes(s->data(), s->size());
	}
}


void TesWriter::writeTagTables()
{
	gatherSharedItems(tile_.tagTables(), 1, 127);
	out_.writeVarint(sharedElements_.size());
	for (const auto& e : sharedElements_)
	{
		writeTagTable(static_cast<TTagTable*>(e));
	}
}


void TesWriter::writeRelationTables()
{
	gatherSharedItems(tile_.relationTables(), 1, 63);
	out_.writeVarint(sharedElements_.size());
	for (const auto& e : sharedElements_)
	{
		writeRelationTable(static_cast<TRelationTable*>(e));
	}
}

// TODO: let caller clear sharedElements_?
template <typename T>
void TesWriter::gatherSharedItems(const ElementDeduplicator<T>& items, int minUsers, size_t firstGroupSize)
{
	assert (firstGroupSize == 127 || firstGroupSize == 63);
	sharedElements_.clear();
	ElementDeduplicator<TSharedElement>::Iterator iter = items.iter();
	while (iter.hasNext())
	{
		TSharedElement* item = iter.next();
		if (item->users() > minUsers) sharedElements_.push_back(item);
	}

	std::sort(sharedElements_.begin(), sharedElements_.end(),
		[](const TSharedElement* a, const TSharedElement* b)
		{
			// Sort in descending order based on number of users
			return a->users() > b->users();
		});

	size_t start = 0;
	size_t end = std::min(firstGroupSize + 1, shared.size());
	for (;;)
	{
		// Within each group, sort elements in their natural order
		std::sort(sharedElements_.begin() + start, sharedElements_.begin() + end);
		start = end;
		end = std::min(end * 128, shared.size());
	}

	for (int i = 0; i < sharedElements_.size(); i++)
	{
		sharedElements_[i]->setLocation(i + 1);
	}
}



uint32_t TesWriter::getTagValue(pointer p, int valueFlags)
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
	return value;
}

void TesWriter::writeTagTable(const TTagTable* tags)
{
	pointer pTable = tags->data() + tags->anchor();
	pointer p;
	if (tags->hasLocalTags())
	{
		pointer origin = pointer::ofTagged(pTable, 0xffff'ffff'ffff'fffcULL);
		p = pTable;
		for (;;)
		{
			p -= 4;
			int32_t key = p.getUnalignedInt();
			int flags = key & 7;
			pointer pKeyString = origin + ((key ^ flags) >> 1);
			TString* keyStr = tile_.getString(pKeyString);
			int valueType = flags & 3;
			p -= 2 + (valueType & 2);
			localKeyTags_.push_back(Tag(
				(keyStr->location() << 2) | valueType,
				getTagValue(p, valueType)));
			if (flags & 4) break;  // last-tag?
		}

		out_.writeVarint(localKeyTags_.size());
		for (Tag tag : localKeyTags_)
		{
			out_.writeVarint(tag.key);
			out_.writeVarint(tag.value);
		}
		localKeyTags_.clear();
	}

	p = pTable;
	if (p.getUnalignedUnsignedInt() == TagsRef::EMPTY_TABLE_MARKER)
	{
		// TODO: not needed in the future, treated as normal tag
		out_.writeByte(0);
		out_.writeByte(0);
	}
	else
	{
		uint32_t prevKey = 0;
		for (;;)
		{
			uint32_t keyBits = p.getUnsignedShort();
			uint32_t key = (keyBits & 0x7fff) >> 2;
			int valueType = keyBits & 3;
			out_.writeVarint(((key - prevKey) << 2) | valueType);
			p += 2;
			out_.writeVarint(getTagValue(p, valueType));
			p += 2 + (keyBits & 2);
			prevKey = key;
			if (keyBits & 0x8000) break;
		}
	}
}


void TesWriter::writeFeatures()
{
	for (const auto& feature : features_)
	{
		switch (feature.typeCode())
		{
		case 0:
			writeNode(reinterpret_cast<TNode*>(feature.feature()));
			break;
		case 1:
			writeWay(reinterpret_cast<TWay*>(feature.feature()));
			break;
		case 2:
			writeRelation(reinterpret_cast<TRelation*>(feature.feature()));
			break;
		}
	}
}

void TesWriter::writeStub(const TFeature* feature, int flags)
{
	TTagTable* tags = feature->tags(tile_);
	flags |= TesFlags::TAGS_CHANGED | TesFlags::GEOMETRY_CHANGED;
	flags |= tags->location() ? TesFlags::SHARED_TAGS : 0;
	flags |= feature->isRelationMember() ? TesFlags::RELATIONS_CHANGED : 0;
	out_.writeByte(flags);

	if (flags & TesFlags::SHARED_TAGS)
	{
		out_.writeVarint(tags->location());
	}
	else
	{
		writeTagTable(tags);
	}

	if (flags & TesFlags::RELATIONS_CHANGED)
	{
		TRelationTable* rels = tile_.getRelationTable(
			feature->feature().relationTableFast());
		if (rels->location())
		{
			out_.writeVarint(rels->location());
		}
		else
		{
			writeRelationTable(rels);
		}
	}
}


void TesWriter::writeNode(const TNode* node)
{
	NodeRef nodeRef(node->feature());
	bool isRelationMember = nodeRef.isRelationMember();
	int flags = (node->feature().flags() & FeatureFlags::WAYNODE) ?
		TesFlags::NODE_BELONGS_TO_WAY : 0;
	writeStub(node, flags);  // TODO: has_shared_location / is_exception_node  

	Coordinate xy = nodeRef.xy();
	out_.writeSignedVarint(static_cast<int64_t>(xy.x) - prevXY_.x);
	out_.writeSignedVarint(static_cast<int64_t>(xy.y) - prevXY_.y);
	prevXY_ = xy;
}


void TesWriter::writeWay(const TWay* way)
{
	WayRef wayRef(way->feature());
	bool hasFeatureNodes = (wayRef.flags() & FeatureFlags::WAYNODE);
	int flags = 
		(hasFeatureNodes ? TesFlags::MEMBERS_CHANGED : 0) |
		(wayRef.isArea() ? TesFlags::IS_AREA : 0) |
		TesFlags::NODE_IDS_CHANGED;
	writeStub(way, flags);
	
	pointer pBody = way->body().data();
	int anchor = way->body().anchor();

	Coordinate xy = wayRef.bounds().bottomLeft();
	out_.writeSignedVarint(static_cast<int64_t>(xy.x) - prevXY_.x);
	out_.writeSignedVarint(static_cast<int64_t>(xy.y) - prevXY_.y);
	prevXY_ = xy;

	out_.writeBytes(pBody.asBytePointer() + anchor, way->body().size() - anchor);

	if (hasFeatureNodes)
	{
		int skipReltablePointer = (wayRef.flags() & FeatureFlags::RELATION_MEMBER) ? 4 : 0;
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
	int flags =
		(relationRef.isArea() ? TesFlags::IS_AREA : 0) |
		TesFlags::MEMBERS_CHANGED | TesFlags::BBOX_CHANGED;
	writeStub(relation, flags);

	// LOG("Writing relation/%lld", relationRef.id());

	const TRelationBody& body = relation->body();
	pointer pBody = body.data();

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


void TesWriter::writeRelationTable(const TRelationTable* relTable)
{
	out_.writeVarint(relTable->size());
	pointer p = relTable->data();
	for (;;)
	{
		uint32_t rel = p.getUnalignedInt();
		p += 4;		
			// TODO: When we switch to TEX, <rel> could be 2 or 4 bytes
		if (rel & MemberFlags::FOREIGN)
		{
			out_.writeVarint((rel >> 3) |  // TODO: changes to TEX_DELTA in future, >> 4
				(rel & MemberFlags::DIFFERENT_TILE) ? 1 : 0);
			if (rel & MemberFlags::DIFFERENT_TILE)
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
				out_.writeSignedVarint(tipDelta >> 1);     // signed
			}
		}
		else
		{
			TReferencedElement* relation = tile_.getElement(p + (rel & 0xffff'fffc));
			assert(relation->location());
			out_.writeVarint((relation->location() - nodeCount_ - wayCount_) << 1);
			// *Relation* number, not feature number
			// Bit 0 is always clear
		}
		if (rel & MemberFlags::LAST) break;
	}
}
