// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#include "Tags.h"
#include <common/util/StringBuilder.h>

#ifdef GEODESK_PYTHON
PyObject* TagsRef::getValue(PyObject* key, StringTable& strings) const
{
	int64_t value = getKeyValue(key, strings);
	return valueAsObject(value, strings);
}

int64_t TagsRef::getKeyValue(PyObject* key, const StringTable& strings) const
{
	int code = strings.getCode(key);
	if (code >= 0 && code <= MAX_COMMON_KEY)
	{
		return getGlobalKeyValue(code);
	}
	const char* str;
	Py_ssize_t len;
	str = PyUnicode_AsUTF8AndSize(key, &len);
	return getLocalKeyValue(str, len);
}

#endif

int64_t TagsRef::getKeyValue(const char* key, int len,
	const StringTable& strings) const
{
	int code = strings.getCode(key, len);
	if (code >= 0 && code <= MAX_COMMON_KEY)
	{
		return getGlobalKeyValue(code);
	}
	return getLocalKeyValue(key, len);
}

TagBits TagsRef::getLocalKeyValue(const char* key, int len) const
{
	if (!hasLocalKeys()) return 0;
	pointer p = pointer::ofTagged(taggedPtr_, ~1);
	pointer origin = pointer::ofTagged(taggedPtr_, ~3);
	p -= 6;
	for (; ; )
	{
		TagBits tag = p.getUnalignedLong();
		int32_t rawPointer = static_cast<int32_t>(tag >> 16);
		int32_t flags = rawPointer & 7;
		// uncommon keys are relative to the 4-byte-aligned tagtable address
		LocalString keyString(origin + ((rawPointer ^ flags) >> 1));
		if (keyString.equals(key, len))
		{
			return (static_cast<TagBits>(p - taggedPtr_ - 2) << 32) |
				((tag & 0xffff) << 16) | flags;
		}
		if (flags & 4) return 0;
		p -= 6 + (flags & 2);
	}
}

int64_t TagsRef::getGlobalKeyValue(int key) const
{
	uint16_t keyBits = key << 2;
	pointer pTable = pointer::ofTagged(taggedPtr_, ~1);
	pointer p = pTable;
	for (; ; )
	{
		// TODO: maybe just fetch key/value separately
		// to avoid unaligned read issue altogether
		uint32_t tag = p.getUnalignedUnsignedInt();
		if ((tag & 0xffff) >= keyBits)
		{
			if ((tag & 0x7ffc) != keyBits) return 0;
			return (static_cast<TagBits>(p - taggedPtr_ + 2) << 32) | tag;
		}
		p += 4 + (tag & 2);
	}
}


int32_t TagsRef::narrowNumber(TagBits value)
{
	assert((value & 3) == 0);
	return static_cast<int32_t>(static_cast<uint32_t>(value) >> 16) + MIN_NUMBER;
}

double TagsRef::wideNumber(TagBits value) const
{
	assert((value & 3) == 2);
	pointer pValue(taggedPtr_ + (value >> 32));
	return TagValue::doubleFromWideNumber(pValue.getUnsignedInt());
}

#ifdef GEODESK_PYTHON
PyObject* TagsRef::valueAsString(TagBits value, StringTable& strings) const
{
	if (value == 0) return strings.getStringObject(0);
	int typeAndSize = (int)value & 3;
	if (typeAndSize == 1) return getGlobalStringObject(value, strings);
	if (typeAndSize == 3) return getLocalStringObject(value);
	if (typeAndSize == 0)
	{
		return PyUnicode_FromFormat("%d", narrowNumber(value));
	}
	assert(typeAndSize == 2);

	pointer pValue(taggedPtr_ + (value >> 32));
	uint32_t rawValue = pValue.getUnsignedInt();
	int scale = rawValue & 3;
	StringBuilder buf;
	buf.formatDouble(TagValue::doubleFromWideNumber(rawValue), scale, true);
	return buf.toPythonString();
}

PyObject* TagsRef::valueAsObject(TagBits value, StringTable& strings) const
{
	if (value == 0)
	{
		Py_INCREF(Py_None);
		return Py_None;
	}
	int typeAndSize = (int)value & 3;
	if (typeAndSize == 1) return getGlobalStringObject(value, strings);
	if (typeAndSize == 3) return getLocalStringObject(value);
	if (typeAndSize == 0)
	{
		return PyLong_FromLong(narrowNumber(value));
	}
	assert(typeAndSize == 2);
	return PyFloat_FromDouble(wideNumber(value));
	// TODO: use PyLong_FromLong if scale == 0
}


PyObject* TagsRef::valueAsNumber(TagBits value, StringTable& strings) const
{
	if (value == 0) return PyLong_FromLong(0);
	int typeAndSize = (int)value & 3;
	if (typeAndSize == 0)
	{
		return PyLong_FromLong(narrowNumber(value));
	}
	if (typeAndSize == 2)
	{
		return PyFloat_FromDouble(wideNumber(value));
	}
	if (typeAndSize == 3)
	{
		double v = localString(value).toDouble();
		return PyFloat_FromDouble(v);
	}
	assert(typeAndSize == 0);	// global string
	double v = globalString(value, strings).toDouble();
	return PyFloat_FromDouble(v);
}

#endif // GEODESK_PYTHON

uint32_t TagsRef::count() const
{
	int count = 0;
	uint32_t tag;
	pointer pTable = pointer::ofTagged(taggedPtr_, ~1);
	if (pTable.getUnalignedUnsignedInt() != EMPTY_TABLE_MARKER)
	{
		pointer p = pTable;
		do
		{
			count++;
			tag = p.getUnalignedUnsignedInt();
			p += 4 + (tag & 2);
		}
		while((tag & 0x8000) == 0);
	}
	if (hasLocalKeys())
	{
		pointer p = pTable - 4;
		do
		{
			count++;
			tag = p.getUnalignedUnsignedInt();
			p -= 6 + (tag & 2);
		}
		while ((tag & 4) == 0);
	}
	return count;
}
