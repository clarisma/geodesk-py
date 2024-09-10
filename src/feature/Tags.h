// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include <cassert>
#include <string>
#include <string_view>
#ifdef GEODESK_PYTHON
#include <Python.h>
#include "python/util/util.h"
#endif
#include "types.h"
#include "StringTable.h"
#include "api/TagValue.h"
#include <common/compile/unreachable.h>
#include <common/util/pointer.h>
#include <geos/vend/json.hpp>

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

class TagsRef
{
public:
	TagsRef(pointer ppTags) { taggedPtr_ = ppTags + ppTags.getInt(); }
	TagsRef(const uint64_t* ppTags) : TagsRef(pointer(ppTags)) {}

	static TagsRef empty() { return TagsRef(&EMPTY_TABLE_STRUCT); }

	uint32_t count() const;
	TagBits getKeyValue(PyObject* key, const StringTable& strings) const;
	TagBits getKeyValue(const char* key, size_t len,
		const StringTable& strings) const;
	TagBits getKeyValue(const std::string_view& key, const StringTable& strings) const
	{
		return getKeyValue(key.data(), key.size(), strings);
	}
	TagBits getGlobalKeyValue(int keyCode) const;
	TagBits getLocalKeyValue(const char* key, int len) const;
	bool hasLocalKeys() const
	{
		return reinterpret_cast<std::uintptr_t>(taggedPtr_) & 1;
	}

	#ifdef GEODESK_PYTHON
	PyObject* valueAsString(TagBits value, StringTable& strings) const;
	PyObject* valueAsObject(TagBits value, StringTable& strings) const;
	PyObject* valueAsNumber(TagBits value, StringTable& strings) const;
	PyObject* getValue(PyObject* key, StringTable& strings) const;
	#endif

	geodesk::TagValue tagValue(TagBits value, StringTable& strings) const
	{
		// TODO: if strings were 0/2 instead of 1/3,
		//  we would not need this check
		//  Tag-bits value of 0 would instead translate to global-string #0 (empty string),
		//  which is the default if a key is not found
		//  Or, getGlobalKeyValue() could return `1` to signify "tag not found",
		//  which is less intuitive

		if (value == 0) return {};

		int typeAndSize = (int)value & 3;
		switch(typeAndSize)
		{
		case 0:	// narrow number
			return {(static_cast<uint32_t>(value) >> 16) << 2};
		case 1:	// global string
			return {1, strings.getGlobalString(static_cast<uint32_t>(value) >> 16)};
		case 2: // wide number
		{
			pointer pValue(taggedPtr_ + (value >> 32));
			return { (pValue.getUnalignedUnsignedInt() << 2) | 2};
		}
		case 3: // local string
		{
			pointer ppValue(taggedPtr_ + (value >> 32));
			return {3, reinterpret_cast<const ShortVarString*>(
				ppValue.asBytePointer() + ppValue.getUnalignedInt())};
		}
		default:
			UNREACHABLE_CASE
		}
	}

	pointer ptr() const { return pointer::ofTagged(taggedPtr_, -2); }
	pointer taggedPtr() const { return taggedPtr_; }
	pointer alignedBasePtr() const { return pointer::ofTagged(taggedPtr_, -4); }
	int32_t pointerOffset(pointer p) const { return p - taggedPtr_; }
	// This is always based off the tagged pointer , not the actual pointer

	static const int MAX_COMMON_KEY = (1 << 13) - 2;
	static const int MIN_NUMBER = -256;		// TODO: duplicated in TagValue namespace
	static const int MAX_WIDE_NUMBER = (1 << 30) - 1 + MIN_NUMBER;
	static const int MAX_NARROW_NUMBER = (1 << 16) - 1 + MIN_NUMBER;
	// TODO: duplicated in TagValue!

	// TODO: This value will change in v2!
	static constexpr uint32_t EMPTY_TABLE_MARKER = 0xffff'ffff;
	static const uint32_t EMPTY_TABLE_STRUCT[2];

private:
	// Note: wideNumber / localString rely on tagPtr_, hence cannot be static
	static int32_t narrowNumber(int64_t value);
	Decimal wideNumber(TagBits value) const;

	static const ShortVarString* globalString(TagBits value, StringTable& strings)
	{
		assert((value & 3) == 1);
		return strings.getGlobalString(static_cast<uint32_t>(value) >> 16);
	}

	const ShortVarString* localString(TagBits value) const
	{
		assert((value & 3) == 3);
		pointer ppValue(taggedPtr_ + (value >> 32));
		return reinterpret_cast<const ShortVarString*>(ppValue.asBytePointer() + ppValue.getUnalignedInt());
	}

	#ifdef GEODESK_PYTHON
	static PyObject* getGlobalStringObject(TagBits value, StringTable& strings)
	{
		assert((value & 3) == 1);
		return strings.getStringObject(static_cast<uint32_t>(value) >> 16);
	}

	PyObject* getLocalStringObject(TagBits value) const
	{
		return Python::toStringObject(*localString(value));
	}
	#endif

	const uint8_t* taggedPtr_;

	friend class FeatureWriter;
};
