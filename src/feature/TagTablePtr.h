// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include <cassert>
#include <string>
#ifdef GEODESK_PYTHON
#include <Python.h>
#endif
#include "types.h"
#include "StringTable.h"
#include <common/util/DataPtr.h>
#include <common/util/Strings.h>
#include <common/util/TaggedPtr.h>

// TODO -- WIP

/*
* Bit 0 : type (0 = number, 1 = string)
* Bit 1 : size flag (0 = narrow, 1 = wide)
* Bit 2 - 15 : unused
* Bits 16 - 31 : narrow value (if size flag = 0)
* Bits 32 - 63 : relative pointer to wide value (if size flag = 1)
*
* 0 = no tag value found
*/
typedef int64_t TagBits;

class TagTablePtr
{
public:
	TagTablePtr(DataPtr p, bool hasLocalTags) :
		TagTablePtr(TaggedPtr<const uint8_t, 1>(p, hasLocalTags)) {}

	static TagTablePtr readFrom(DataPtr ppTags)
	{
		return TagTablePtr(TaggedPtr<const uint8_t, 1>(ppTags + ppTags.getInt()));
	}

	uint32_t count() const;
	TagBits getKeyValue(PyObject* key, const StringTable& strings) const;
	TagBits getKeyValue(const char* key, int len,
		const StringTable& strings) const;
	TagBits getGlobalKeyValue(int keyCode) const;
	TagBits getLocalKeyValue(const char* key, int len) const;
	bool hasLocalKeys() const
	{
		return taggedPtr_.flags();
	}

	#ifdef GEODESK_PYTHON
	PyObject* valueAsString(TagBits value, StringTable& strings) const;
	PyObject* valueAsObject(TagBits value, StringTable& strings) const;
	PyObject* valueAsNumber(TagBits value, StringTable& strings) const;
	PyObject* getValue(PyObject* key, StringTable& strings) const;
	#endif

	DataPtr ptr() const { return taggedPtr_.ptr(); }
	TaggedPtr<const uint8_t, 1> taggedPtr() const { return taggedPtr_; }
	DataPtr alignedBasePtr() const 
	{ 
		return taggedPtr_.raw() & 0xffff'ffff'ffff'fffcULL; 
	}
	// int32_t pointerOffset(pointer p) { return p - taggedPtr_; }
	// This is always based off the tagged pointer , not the actual pointer

	// TODO: This value will change in 2.0!
	static const uint32_t EMPTY_TABLE_MARKER = 0xffff'ffff;
	static const uint32_t EMPTY_TABLE_STRUCT[2];

private:
	TagTablePtr(TaggedPtr<const uint8_t,1> taggedPtr) : taggedPtr_(taggedPtr) {}

	// Note: wideNumber / localString rely on tagPtr_, hence cannot be static
	static int32_t narrowNumber(int64_t value);
	double wideNumber(TagBits value) const;

	static GlobalString globalString(TagBits value, StringTable& strings)
	{
		assert((value & 3) == 1);
		return strings.getGlobalString(static_cast<uint32_t>(value) >> 16);
	}

	LocalString localString(TagBits value) const
	{
		assert((value & 3) == 3);
		pointer ppValue(taggedPtr_ + (value >> 32));
		return LocalString(ppValue + ppValue.getUnalignedInt());
	}

	#ifdef GEODESK_PYTHON
	static PyObject* getGlobalStringObject(TagBits value, StringTable& strings)
	{
		assert((value & 3) == 1);
		return strings.getStringObject(static_cast<uint32_t>(value) >> 16);
	}

	PyObject* getLocalStringObject(TagBits value) const
	{
		return localString(value).toStringObject();
	}
	#endif

	TaggedPtr<const uint8_t, 1> taggedPtr_;

	friend class FeatureWriter;
};



class GlobalTagIterator
{
public:
	GlobalTagIterator(TagTablePtr tags) :
		pNextTag_(tags.ptr()),
		keyBits_(0),
		value_(0)
	{
	}

	bool next()
	{
		pCurrentTag_ = pNextTag_;
		if (keyBits_ & 0x8000) return false;
		keyBits_ = pCurrentTag_.getUnsignedShort();
		pNextTag_ += (keyBits_ & 2) + 4;
		
		// TODO: not needed in 2.0
		if (keyBits_ == 0xffff) keyBits_ = 0x8000;

		if (keyBits_ & 2)
		{
			value_ = (pCurrentTag_ + 2).getUnsignedIntUnaligned();
		}
		else
		{
			value_ = (pCurrentTag_ + 2).getUnsignedShort();
		}
		return true;
	}

	uint32_t keyBits() const { return keyBits_; }
	uint32_t key() const { return (keyBits_ >> 2) & 0x1fff; }
	bool hasStringValue() const { return keyBits_ & 1; }
	bool hasWideValue() const { return keyBits_ & 2; }
	bool hasLocalStringValue() const { return (keyBits_ & 3) == 3; }
	uint32_t value() const { return value_; }
	DataPtr stringValueFast() const 
	{ 
		assert(hasLocalStringValue());
		return pCurrentTag_ + 2 + static_cast<int32_t>(value_);
	}
	DataPtr ptr() const { return pCurrentTag_; }

private:
	DataPtr pCurrentTag_;
	DataPtr pNextTag_;
	uint32_t keyBits_;
	uint32_t value_;
};


class LocalTagIterator
{
public:
	LocalTagIterator(TagTablePtr tags) :
		p_(tags.ptr()),
		pOrigin_(tags.alignedBasePtr()),
		keyBits_(tags.hasLocalKeys() ? 0 : 4),
		value_(0)
	{
	}

	bool next()
	{
		if (keyBits_ & 4) return false;
		p_ -= 4;
		keyBits_ = p_.getIntUnaligned();	// signed
		int flags = keyBits_ & 7;
		pKeyString_ = pOrigin_ + ((keyBits_ ^ flags) >> 1);
		p_ -= 2 + (keyBits_ & 2);
		if (keyBits_ & 2)
		{
			value_ = p_.getUnsignedIntUnaligned();
		}
		else
		{
			value_ = p_.getUnsignedShort();
		}
		return true;
	}

	int32_t keyBits() const { return keyBits_; }
	DataPtr keyString() const { return pKeyString_; }
	bool hasStringValue() const { return keyBits_ & 1; }
	bool hasWideValue() const { return keyBits_ & 2; }
	bool hasLocalStringValue() const { return (keyBits_ & 3) == 3; }
	uint32_t value() const { return value_; }
	DataPtr stringValueFast() const 
	{ 
		assert(hasLocalStringValue());
		return p_ + static_cast<int32_t>(value_);
	}
	DataPtr ptr() const { return p_; }

private:
	DataPtr p_;
	DataPtr pOrigin_;
	int32_t keyBits_;		// must be signed for locals
	DataPtr pKeyString_;
	uint32_t value_;
};
