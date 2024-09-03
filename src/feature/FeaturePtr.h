// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include <common/util/DataPtr.h>
#include <common/util/log.h>
#include "TagTablePtr.h"
#include "geom/Box.h"
#include "geom/Coordinate.h"

class FeatureStore;
class Matcher;
class MatcherHolder;
class Filter;
class StringTable;


class FeatureHeader
{
public:
	FeatureHeader(uint64_t bits) : bits_(bits) {}

	// TODO: This will change in 2.0
	static FeatureHeader forTypeAndId(int type, uint64_t id)
	{
		uint64_t hi = (((id >> 32) << 8) | (type << 3));
		uint64_t lo = id << 32;
		return FeatureHeader(hi | lo);
	}

	uint64_t bits() const noexcept { return bits_; }

	// TODO: This will change in 2.0
	uint64_t id() const noexcept
	{
		uint32_t hi = static_cast< uint32_t>(bits_) >> 8;
		uint32_t lo = static_cast<uint32_t>(bits_ >> 32);
		return (static_cast<uint64_t>(hi) << 32) | lo;
	}

	// TODO: This will change in 2.0
	int flags() const noexcept 
	{ 
		return static_cast<int>(bits_);
	}

	int typeCode() const
	{
		return static_cast<int>(bits_ >> 3) & 3;
	}

private:
	uint64_t bits_;
};



class FeaturePtr
{
public:
	FeaturePtr() {}
	FeaturePtr(const uint8_t* p) : p_(p) {}
	FeaturePtr(const FeaturePtr& other) : p_(other.p_) {}

	operator DataPtr () const noexcept { return p_; }

	FeaturePtr& operator=(const FeaturePtr& other) noexcept
	{
		p_ = other.p_;
		return *this;
	}

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

	FeatureHeader header()
	{
		return FeatureHeader(p_.getUnsignedLongUnaligned());
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

	TagTablePtr tags() const
	{
		return TagTablePtr::readFrom(p_ + 8);
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
		assert(isRelationMember());
		// For nodes, the body pointer points to the reltable
		// For ways and relations, the reltable pointer sits right
		// before the body anchor
		DataPtr ppRelTable = isNode() ? (p_ + 12) : (bodyptr() - 4);
		return ppRelTable.followUnaligned();
			// TODO: unaligned only for ways/relations
	}

	int32_t relationTableHandleFast(const uint8_t* base) const
	{
		LOG("Getting relations of %s", toString().c_str());
		LOG("  feature ptr = %p", p_);
		LOG("     base ptr = %p", base);
		assert(isRelationMember());
		// For nodes, the body pointer points to the reltable
		// For ways and relations, the reltable pointer sits right
		// before the body anchor
		LOG("body ptr = %p", bodyptr().ptr());
		DataPtr ppRelTable = isNode() ? (p_ + 12) : (bodyptr() - 4);
		LOG("ppRelTable = %p", ppRelTable.ptr());
		LOG("relative pointer = %d", ppRelTable.getIntUnaligned());
		LOG("feature handle = %d", (int)(p_ - base));
		return (ppRelTable - base) + ppRelTable.getIntUnaligned();
	}

	/**
	 * A 64-bit value that uniquely identifies the feature based on its
	 * type and ID.
	 * 
	 * TODO: Changes in 2.0
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
	DataPtr p_;
};


struct FeatureStruct
{
	Box bounds;
	FeatureHeader header;
	int32_t tagTablePtr;		// tagged
	int32_t relTablePtr;		
};


