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


class AbstractTagIterator
{
public:
	AbstractTagIterator(int_fast32_t handle, TagTablePtr tags) :
		pTile_(tags.ptr() - handle),
		ofs_(handle),
		keyBits_(0)
	{
	}

	AbstractTagIterator(const uint8_t* pTile, TagTablePtr tags) :
		pTile_(pTile),
		ofs_(DataPtr::nearDelta(tags.ptr() - pTile_)),
		keyBits_(0)
	{
	}

	uint32_t keyBits() const { return keyBits_; }
	bool hasStringValue() const { return keyBits_ & 1; }
	bool hasWideValue() const { return keyBits_ & 2; }
	bool hasLocalStringValue() const { return (keyBits_ & 3) == 3; }
	DataPtr ptr() const { return pTile_ + ofs_; }

protected:
	AbstractTagIterator(const uint8_t* pTile, int_fast32_t ofs) :
		pTile_(pTile),
		ofs_(ofs),
		keyBits_(0)
	{
	}

	DataPtr pTile_;
	int_fast32_t ofs_;
	int32_t keyBits_;			// must be signed for locals
};


class GlobalTagIterator : public AbstractTagIterator
{
public:
	using AbstractTagIterator::AbstractTagIterator;

	bool next()
	{
		if (keyBits_ & 0x8000) return false;
		keyBits_ = (pTile_ + ofs_).getUnsignedShort();

		// TODO: not needed in 2.0
		if (keyBits_ == 0xffff) keyBits_ = 0x8000;
		ofs_ += 4 + (keyBits_ & 2);
		return true;
	}

	uint32_t keyBits() const { return static_cast<uint32_t>(keyBits_); }
	uint32_t key() const { return (keyBits() >> 2) & 0x1fff; }
	uint32_t value() const
	{
		// TODO: Could always read an int, then mask off upper bits if narrow
		// Cost of unligned read vs. branch misprediction

		if (keyBits_ & 2)
		{
			return (pTile_ + ofs_ - 4).getUnsignedIntUnaligned();
		}
		else
		{
			return (pTile_ + ofs_ - 2).getUnsignedShort();
		}
	}

	int_fast32_t stringValueHandleFast() const
	{
		assert(hasLocalStringValue());
		int_fast32_t valueOfs = ofs_ - 4;
		return valueOfs + (pTile_ + valueOfs).getIntUnaligned();
	}

	DataPtr stringValueFast() const
	{
		assert(hasLocalStringValue());
		return pTile_ + stringValueHandleFast();
	}
};


class LocalTagIterator : public AbstractTagIterator
{
public:
	LocalTagIterator(int_fast32_t handle, TagTablePtr tags) :
		AbstractTagIterator(tags.ptr() - handle, handle),
		originOfs_(handle & 0xffff'fffc)
	{
		keyBits_ = tags.hasLocalKeys() ? 0 : 4;
	}

	LocalTagIterator(const uint8_t* pTile, TagTablePtr tags) :
		AbstractTagIterator(pTile, DataPtr::nearDelta(tags.ptr() - pTile)),
		originOfs_((DataPtr::nearDelta(tags.ptr() - pTile)) & 0xffff'fffc)
	{
		keyBits_ = tags.hasLocalKeys() ? 4 : 0;
	}

	bool next()
	{
		if (keyBits_ & 4) return false;
		ofs_ -= 4;
		keyBits_ = (pTile_ + ofs_).getIntUnaligned();	// signed
		ofs_ -= 2 + (keyBits_ & 2);
		return true;
	}

	int32_t flags() const { return keyBits_ & 7; }
	
	int_fast32_t keyStringHandle() const
	{
		return originOfs_ + ((keyBits_ >> 1) & 0xffff'fffc);
	}
	
	DataPtr keyString() const 
	{ 
		return pTile_ + keyStringHandle(); 
	}

	uint32_t value() const
	{
		// Cost of unaligned read is likely cheaper than branch misprediction
		return (pTile_ + ofs_).getUnsignedIntUnaligned() & 
			((keyBits_ & 2) ? 0xffff'ffff : 0xffff);
	}

	int_fast32_t stringValueHandleFast() const
	{
		assert(hasLocalStringValue());
		return ofs_ + (pTile_ + ofs_).getIntUnaligned();
	}

	DataPtr stringValueFast() const
	{
		assert(hasLocalStringValue());
		return pTile_ + stringValueHandleFast();
	}

protected:
	int_fast32_t originOfs_;
};

