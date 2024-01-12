// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#include "FeatureWriter.h"
#include "feature/FeatureStore.h"
#include "feature/FastMemberIterator.h"

// TODO: generalize!
void FeatureWriter::writeTagValue(TagsRef tags, TagBits value, StringTable& strings)
{
	if (value & 1) // string
	{
		writeByte('\"');
		if (value & 2)
		{
			writeJsonEscapedString(tags.localString(value).toStringView());
		}
		else
		{
			writeJsonEscapedString(tags.globalString(value, strings).toStringView());
		}
		writeByte('\"');
	}
	else
	{
		if (value & 2)
		{
			formatDouble(tags.wideNumber(value));
		}
		else
		{
			formatInt(TagsRef::narrowNumber(value));
		}
	}
}


void FeatureWriter::writeFeatureGeometry(FeatureStore* store, FeatureRef feature)
{
	if (feature.isWay())
	{
		writeWayGeometry(WayRef(feature));
	}
	else if (feature.isNode())
	{
		writeNodeGeometry(NodeRef(feature));
	}
	else
	{
		assert(feature.isRelation());
		RelationRef relation(feature);
		if (relation.isArea())
		{
			writeAreaRelationGeometry(store, relation);
		}
		else
		{
			writeCollectionRelationGeometry(store, relation);
		}
	}
}

// TODO: In GOL 2.0, empty geometry collections won't exist
//  (no placeholders, no empty relations)

uint64_t FeatureWriter::writeMemberGeometries(FeatureStore* store, RelationRef relation, RecursionGuard& guard)
{
	uint64_t count = 0;
	FastMemberIterator iter(store, relation);
	for (;;)
	{
		FeatureRef member = iter.next();
		if (member.isNull()) break;
		int memberType = member.typeCode();
		if (memberType == 1)
		{
			WayRef memberWay(member);
			if (memberWay.isPlaceholder()) continue;
			if (count > 0)
			{
				writeByte(',');
				if(pretty_) writeByte(' ');
			}
			else
			{
				writeByte(coordGroupStartChar_);
			}
			writeWayGeometry(memberWay);
		}
		else if (memberType == 0)
		{
			NodeRef memberNode(member);
			if (memberNode.isPlaceholder()) continue;
			if (count > 0)
			{
				writeByte(',');
				if (pretty_) writeByte(' ');
			}
			else
			{
				writeByte(coordGroupStartChar_);
			}
			writeNodeGeometry(memberNode);
		}
		else
		{
			assert(memberType == 2);
			RelationRef childRel(member);
			if (childRel.isPlaceholder() || !guard.checkAndAdd(childRel)) continue;
			if (count > 0)
			{
				writeByte(',');
				if (pretty_) writeByte(' ');
			}
			else
			{
				writeByte(coordGroupStartChar_);
			}
			if (childRel.isArea())
			{
				writeAreaRelationGeometry(store, childRel);
			}
			else
			{
				writeCollectionRelationGeometry(store, childRel);
			}
		}
		count++;
	}
	if (count)
	{
		writeByte(coordGroupEndChar_);
	}
	return count;
}
