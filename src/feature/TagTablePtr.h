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
	TagTablePtr(DataPtr ppTags) : taggedPtr_(ppTags + ppTags.getInt()) {}

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
	// pointer alignedBasePtr() const { return pointer::ofTagged(taggedPtr_, -4); }
	// int32_t pointerOffset(pointer p) { return p - taggedPtr_; }
	// This is always based off the tagged pointer , not the actual pointer

	// TODO: This value may change!
	static const uint32_t EMPTY_TABLE_MARKER = 0xffff'ffff;
	static const uint32_t EMPTY_TABLE_STRUCT[2];

private:
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
