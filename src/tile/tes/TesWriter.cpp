// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#include "TesWriter.h"
#include "TesFlags.h"
#include "tile/model/TFeature.h"

TesWriter::TesWriter(TileModel& tile, Buffer* out) :
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
	// writeRelationTables();  // TODO
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
			prevId = 0;		// ID space starts over 
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
	for (auto& e : sharedElements_)
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
	auto iter = items.iter();
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
	size_t end = std::min(firstGroupSize + 1, sharedElements_.size());
	while(start < end)
	{
		// Within each group, sort elements in their natural order
		std::sort(sharedElements_.begin() + start, sharedElements_.begin() + end);
		start = end;
		end = std::min(end * 128, sharedElements_.size());
	}

	for (int i = 0; i < sharedElements_.size(); i++)
	{
		sharedElements_[i]->setLocation(i + 1);
	}
}


/*
void TesWriter::writeTagValue(DataPtr p, int valueFlags)
{
	assert(valueFlags >= 0 && valueFlags <= 3);
	uint32_t value;
	if ((valueFlags & 2) == 0)		  // narrow value
	{
		value = p.getUnsignedShort();
	}
	else                              // wide value
	{
		value = p.getUnsignedIntUnaligned();
		if (valueFlags == 3)
		{
			// TODO:
			TElement::Handle handle = tile_.existingHandle(p + static_cast<int32_t>(value));
			TString* valueStr = tile_.getString(handle);
			assert(valueStr);
			value = valueStr->location();
		}
	}
	out_.writeVarint(value);
}
*/

void TesWriter::writeStringValue(DataPtr pStr)
{
	TElement::Handle handle = tile_.existingHandle(pStr);
	TString* str = tile_.getString(handle);
	assert(str);
	out_.writeVarint(str->location());
}

void TesWriter::writeTagTable(const TTagTable* tags)
{
	TagTablePtr pTags = tags->tags();
	if (tags->hasLocalTags())
	{
		out_.writeVarint(tags->anchor() >> 1);
		LocalTagIterator localTags(pTags);
		while (localTags.next())
		{
			// TODO
			TElement::Handle handle = tile_.existingHandle(localTags.keyString());
			TString* keyStr = tile_.getString(handle);
			assert(keyStr);
			out_.writeVarint((keyStr->location() << 2) | (localTags.keyBits() & 3));
			if (localTags.hasLocalStringValue())
			{
				writeStringValue(localTags.stringValueFast());
			}
			else
			{
				out_.writeVarint(localTags.value());
			}
		}
	}

	uint32_t prevKey = 0;
	GlobalTagIterator globalTags(pTags);
	while (globalTags.next())
	{
		uint32_t key = globalTags.key();
		out_.writeVarint(((key - prevKey) << 2) | (globalTags.keyBits() & 3));
		prevKey = key;
		if (globalTags.hasLocalStringValue())
		{
			writeStringValue(globalTags.stringValueFast());
		}
		else
		{
			out_.writeVarint(globalTags.value());
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
		TRelationTable* rels = feature->parentRelations(tile_);
		if (rels->location())
		{
			out_.writeVarint(rels->location());
		}
		else
		{
			// writeRelationTable(rels);   // TODO
		}
	}
}


void TesWriter::writeNode(const TNode* node)
{
	bool isRelationMember = node->isRelationMember();
	int flags = (node->flags() & FeatureFlags::WAYNODE) ?
		TesFlags::NODE_BELONGS_TO_WAY : 0;
	writeStub(node, flags);  // TODO: has_shared_location / is_exception_node  

	Coordinate xy = node->xy();
	out_.writeSignedVarint(static_cast<int64_t>(xy.x) - prevXY_.x);
	out_.writeSignedVarint(static_cast<int64_t>(xy.y) - prevXY_.y);
	prevXY_ = xy;
}


void TesWriter::writeWay(const TWay* way)
{
	WayPtr wayRef(way->feature());
	bool hasFeatureNodes = (wayRef.flags() & FeatureFlags::WAYNODE);
	int flags = 
		(hasFeatureNodes ? TesFlags::MEMBERS_CHANGED : 0) |
		(wayRef.isArea() ? TesFlags::IS_AREA : 0) |
		TesFlags::NODE_IDS_CHANGED;
	writeStub(way, flags);
	
	DataPtr pBody = way->body().data();
	int anchor = way->body().anchor();

	Coordinate xy = wayRef.bounds().bottomLeft();
	out_.writeSignedVarint(static_cast<int64_t>(xy.x) - prevXY_.x);
	out_.writeSignedVarint(static_cast<int64_t>(xy.y) - prevXY_.y);
	prevXY_ = xy;

	out_.writeBytes(pBody + anchor, way->body().size() - anchor);

	if (hasFeatureNodes)
	{
		int skipReltablePointer = (wayRef.flags() & FeatureFlags::RELATION_MEMBER) ? 4 : 0;
		out_.writeVarint(anchor - skipReltablePointer);
		DataPtr p = pBody + anchor - skipReltablePointer;
		for (;;)
		{
			p -= 4;
			int32_t node = p.getIntUnaligned();
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
				TElement::Handle nodeHandle = tile_.existingHandle(
					p + (static_cast<int32_t>((node >> 2) << 1)));
				TReferencedElement* wayNode = tile_.getElement(nodeHandle);		
				out_.writeVarint(wayNode->location() << 1);
			}
			if (node & MemberFlags::LAST) break;
		}
	}
}

void TesWriter::writeRelation(const TRelation* relation)
{
	RelationPtr relationRef(relation->feature());
	int flags =
		(relationRef.isArea() ? TesFlags::IS_AREA : 0) |
		TesFlags::MEMBERS_CHANGED | TesFlags::BBOX_CHANGED;
	writeStub(relation, flags);

	// LOG("Writing relation/%lld", relationRef.id());

	const TRelationBody& body = relation->body();
	DataPtr pBody = body.data();

	writeBounds(relationRef);

	int anchor = body.anchor();
	out_.writeVarint(body.size() - anchor);

	DataPtr p = pBody + anchor;
	for (;;)
	{
		int32_t member = p.getIntUnaligned();
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
			DataPtr pMember = (p & 0xffff'ffff'ffff'fffcULL) + ((int32_t)(member & 0xffff'fff8) >> 1);
			// TODO: allow new handles as well
			TFeature* memberFeature = reinterpret_cast<TFeature*>(
				tile_.getElement(tile_.existingHandle(pMember)));
			if (memberFeature == nullptr)
			{
				FeaturePtr xxx(pMember);
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
				DataPtr pRoleStr = p + (p.getIntUnaligned() >> 1);
				// TODO: allow new handles as well
				TString* roleStr = tile_.getString(tile_.existingHandle(pRoleStr));
				roleValue = roleStr->location() << 1;
				p += 4;
			}
			out_.writeVarint(roleValue);
		}
		if (member & MemberFlags::LAST) break;
	}
}


void TesWriter::writeBounds(FeaturePtr feature)
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
	DataPtr p = relTable->data();
	for (;;)
	{
		uint32_t rel = p.getIntUnaligned();
			// TODO: When we switch to TEX, <rel> could be 2 or 4 bytes
		if (rel & MemberFlags::FOREIGN)
		{
			p += 4;
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
			// TODO
			DataPtr pRel = p + (rel & 0xffff'fffc);
			TElement::Handle handle = tile_.existingHandle(pRel);
			TReferencedElement* relation = tile_.getRelationTable(handle);
			if (relation) // TODO
			{
				assert(relation);
				assert(relation->location());
				out_.writeVarint((relation->location() - nodeCount_ - wayCount_) << 1);
				// *Relation* number, not feature number
				// Bit 0 is always clear
			}
			p += 4;
		}
		if (rel & MemberFlags::LAST) break;
	}
}
