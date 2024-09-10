// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#include "FeatureWriter.h"
#include <common/util/ShortVarString.h>
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
			writeJsonEscapedString(tags.localString(value)->toStringView());
		}
		else
		{
			writeJsonEscapedString(tags.globalString(value, strings)->toStringView());
		}
		writeByte('\"');
	}
	else
	{
		if (value & 2)
		{
			Decimal d = tags.wideNumber(value);
			char buf[64];
			char* p = d.format(buf);
			writeBytes(buf, p - buf);
		}
		else
		{
			formatInt(TagsRef::narrowNumber(value));
		}
	}
}


void FeatureWriter::writeFeatureGeometry(FeatureStore* store, FeaturePtr feature)
{
	if (feature.isWay())
	{
		writeWayGeometry(WayPtr(feature));
	}
	else if (feature.isNode())
	{
		writeNodeGeometry(NodePtr(feature));
	}
	else
	{
		assert(feature.isRelation());
		RelationPtr relation(feature);
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

uint64_t FeatureWriter::writeMemberGeometries(FeatureStore* store, RelationPtr relation, RecursionGuard& guard)
{
	uint64_t count = 0;
	FastMemberIterator iter(store, relation);
	for (;;)
	{
		FeaturePtr member = iter.next();
		if (member.isNull()) break;
		int memberType = member.typeCode();
		if (memberType == 1)
		{
			WayPtr memberWay(member);
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
			NodePtr memberNode(member);
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
			RelationPtr childRel(member);
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


void FeatureWriter::writeDefaultId(FeatureWriter* writer,
	FeatureStore* store, FeaturePtr feature, void* closure)
{
	char quoteChar = writer->quoteChar_;
	if (quoteChar) writer->writeByte(quoteChar);
	writer->writeByte("NWRS"[feature.typeCode()]);
	writer->formatInt(feature.id());		// TODO: rename, is long in reality
	if (quoteChar) writer->writeByte(quoteChar);

}


void FeatureWriter::writeId(FeatureStore* store, FeaturePtr feature)
{
	writeIdFunction_(this, store, feature, writeIdClosure_);
}
