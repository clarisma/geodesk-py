// Copyright (c) 2023 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include "Tags.h"
#include "geom/Box.h"
#include "geom/Coordinate.h"

/*
struct FeatureStub
{
	Box box;
	uint32_t highIdAndFlags;
	uint32_t lowId;
	int32_t	 tagsPointer;
	int32_t	 bodyPointer;
};
*/

class FeatureStore;
class Matcher;
class MatcherHolder;
class Filter;
class StringTable;


class FeatureRef
{
public:
	FeatureRef(pointer p) { ptr_ = p; }

	uint64_t id() const
	{
		uint32_t hi = ptr_.getUnsignedInt() >> 8;
		uint32_t lo = ptr_.getUnsignedInt(4);
		return (static_cast<uint64_t>(hi) << 32) | lo;
	}

	// TODO: should just get pointer
	Box bounds() const
	{
		return Box(ptr_.getInt(-16), ptr_.getInt(-12), ptr_.getInt(-8), ptr_.getInt(-4));
	}

	bool isNull() const	{ return !ptr_; }
	int  flags() const 	{ return ptr_.getInt();	}
	bool isArea() const	{ return ptr_.getUnsignedInt() & FeatureFlags::AREA; }
	bool isNode() const	{ return typeCode() == FeatureType::NODE; }
	bool isWay() const	{ return typeCode() == FeatureType::WAY; }
	bool isRelation() const	{ return typeCode() == FeatureType::RELATION; }
	bool isType(FeatureTypes types) { return types.acceptFlags(flags()); }

	TagsRef tags() const
	{
		pointer ppTags(ptr_ + 8);
		return TagsRef(ppTags);
	}

	pointer ptr() const { return ptr_; }
	pointer bodyptr() const { return ptr_.follow(12); }
	const uint8_t* asBytePointer() const { return ptr_.asBytePointer();	}

	/**
	 * Obtains a pointer to the feature's relation table.
	 * The feature must belong to a relation, otherwise the
	 * result is undefined.
	 */
	pointer relationTableFast() const
	{
		assert(flags() & FeatureTypes::RELATION_MEMBERS);
		// For nodes, the body pointer points to the reltable
		// For ways and relations, the reltable pointer sits right
		// before the body anchor
		pointer ppRelTable = isNode() ? (ptr_ + 12) : (bodyptr() - 4);
		return ppRelTable.follow();
	}

	/**
	 * A 64-bit value that uniquely identifies the feature based on its
	 * type and ID.
	 */
	int64_t idBits() const
	{
		return ptr_.getLong() & 0xffff'ffff'ffff'ff18;
	}

	int64_t hash() const
	{
		return idBits();
	}

	int typeCode() const
	{
		return (ptr_.getUnsignedInt() >> 3) & 3;
	}

	// TODO: not valid for NodeRef
	int32_t minX() const { return ptr_.getInt(-16); }
	int32_t minY() const { return ptr_.getInt(-12); }
	int32_t maxX() const { return ptr_.getInt(-8); }
	int32_t maxY() const { return ptr_.getInt(-4); }

	const char* typeName() const;
	std::string toString() const;

protected:
	pointer ptr_;
};



