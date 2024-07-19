// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include <common/util/DataPtr.h>
#include "Tags.h"
#include "geom/Box.h"
#include "geom/Coordinate.h"

class FeatureStore;
class Matcher;
class MatcherHolder;
class Filter;
class StringTable;


class FeaturePtr
{
public:
	FeaturePtr(uint8_t* p) { p_ = p; }

	uint64_t id() const noexcept
	{
		uint32_t hi = p_.getUnsignedInt() >> 8;
		uint32_t lo = (p_ + 4).getUnsignedInt();
		return (static_cast<uint64_t>(hi) << 32) | lo;
	}

	uint64_t typedId() const noexcept
	{
		return (id() << 2) | typeCode();
	}

	// TODO: should just get pointer
	Box bounds() const noexcept
	{
		assert(!isNode());
		return Box((p_-16).getInt(), (p_-12).getInt(), (p_-8).getInt(), (p_-4).getInt());
	}

	bool isNull() const	{ return !p_; }
	int  flags() const 	{ return p_.getInt();	}
	bool isArea() const	{ return p_.getUnsignedInt() & FeatureFlags::AREA; }
	bool isNode() const	{ return typeCode() == FeatureType::NODE; }
	bool isWay() const	{ return typeCode() == FeatureType::WAY; }
	bool isRelation() const	{ return typeCode() == FeatureType::RELATION; }
	bool isRelationMember() const { return flags() & FeatureFlags::RELATION_MEMBER; }
	bool isType(FeatureTypes types) { return types.acceptFlags(flags()); }
	bool intersects(const Box& bounds)
	{
		assert(!isNode());
		return (!((p_-16).getInt() > bounds.maxX() ||
			(p_-12).getInt() > bounds.maxY() ||
			(p_-8).getInt() < bounds.minX() ||
			(p_-4).getInt() < bounds.minY()));
	}

	TagsRef tags() const
	{
		pointer ppTags(p_ + 8);
		return TagsRef(ppTags);
	}

	DataPtr ptr() const { return p_; }
	DataPtr bodyptr() const { return (p_ + 12).follow(); }
	
	/**
	 * Obtains a pointer to the feature's relation table.
	 * The feature must belong to a relation, otherwise the
	 * result is undefined.
	 */
	DataPtr relationTableFast() const
	{
		assert(flags() & FeatureTypes::RELATION_MEMBERS);
		// For nodes, the body pointer points to the reltable
		// For ways and relations, the reltable pointer sits right
		// before the body anchor
		DataPtr ppRelTable = isNode() ? (p_ + 12) : (bodyptr() - 4);
		return ppRelTable.follow();
	}

	/**
	 * A 64-bit value that uniquely identifies the feature based on its
	 * type and ID.
	 */
	int64_t idBits() const
	{
		return p_.getLong() & 0xffff'ffff'ffff'ff18;
	}

	int64_t hash() const
	{
		return idBits();
	}

	int typeCode() const
	{
		return (p_.getUnsignedInt() >> 3) & 3;
	}

	// TODO: not valid for NodeRef
	int32_t minX() const { return (p_-16).getInt(); }
	int32_t minY() const { return (p_-12).getInt(); }
	int32_t maxX() const { return (p_-8).getInt(); }
	int32_t maxY() const { return (p_-4).getInt(); }

	const char* typeName() const;
	std::string toString() const;

protected:
	const DataPtr p_;
};



