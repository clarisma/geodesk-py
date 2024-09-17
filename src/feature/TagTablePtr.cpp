// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#include "feature/TagTablePtr.h"
#include <common/util/ShortVarString.h>
#include <common/util/StringBuilder.h>

/**
 * A construct that simulates a pointer to an empty tag-table.
 * The first word is a relative pointer to the second word (4),
 * the second word is the empty-table marker itself.
 */
const uint32_t TagValues::EMPTY_TABLE_STRUCT[2] = { 4, TagValues::EMPTY_TABLE_MARKER };

const double TagValues::SCALE_FACTORS[] = { 1.0, 0.1, 0.01, 0.001 };

#ifdef GEODESK_PYTHON
PyObject* TagTablePtr::getValue(PyObject* key, StringTable& strings) const
{
	int64_t value = getKeyValue(key, strings);
	return valueAsObject(value, strings);
}

int64_t TagTablePtr::getKeyValue(PyObject* key, const StringTable& strings) const
{
	int code = strings.getCode(key);
	if (code >= 0 && code <= TagValues::MAX_COMMON_KEY)
	{
		return getGlobalKeyValue(code);
	}
	const char* str;
	Py_ssize_t len;
	str = PyUnicode_AsUTF8AndSize(key, &len);
	return getLocalKeyValue(str, static_cast<int>(len));
}

#endif

int64_t TagTablePtr::getKeyValue(const char* key, size_t len,
	const StringTable& strings) const
{
	int code = strings.getCode(key, len);
	if (code >= 0 && code <= TagValues::MAX_COMMON_KEY)
	{
		return getGlobalKeyValue(code);
	}
	return getLocalKeyValue(key, len);
}

TagBits TagTablePtr::getLocalKeyValue(const char* key, size_t len) const
{
	if (!hasLocalKeys()) return 0;
	DataPtr p = ptr();
	DataPtr origin = alignedBasePtr();
	p -= 6;
	for (; ; )
	{
		TagBits tag = p.getLongUnaligned();
		int32_t rawPointer = static_cast<int32_t>(tag >> 16);
		int32_t flags = rawPointer & 7;
		// uncommon keys are relative to the 4-byte-aligned tagtable address
		const ShortVarString* keyString = reinterpret_cast<const ShortVarString*>
			(origin.ptr() + ((rawPointer ^ flags) >> 1));
		if (keyString->equals(key, len))
		{
			return (static_cast<TagBits>(pointerOffset(p) - 2) << 32) |
				((tag & 0xffff) << 16) | flags;
		}
		if (flags & 4) return 0;
		p -= 6 + (flags & 2);
	}
}

int64_t TagTablePtr::getGlobalKeyValue(int key) const
{
	uint16_t keyBits = key << 2;
	DataPtr pTable = ptr();
	DataPtr p = pTable;
	for (; ; )
	{
		// TODO: maybe just fetch key/value separately
		// to avoid unaligned read issue altogether
		uint32_t tag = p.getUnsignedIntUnaligned();
		if ((tag & 0xffff) >= keyBits)
		{
			if ((tag & 0x7ffc) != keyBits) return 0;
			return (static_cast<TagBits>(pointerOffset(p) + 2) << 32) | tag;
		}
		p += 4 + (tag & 2);
	}
}


int32_t TagTablePtr::narrowNumber(TagBits value) noexcept
{
	assert(valueType(value) == TagValueType::NARROW_NUMBER);
	return static_cast<int32_t>(static_cast<uint32_t>(value) >> 16) + TagValues::MIN_NUMBER;
}

Decimal TagTablePtr::wideNumber(TagBits value) const noexcept
{
	assert(valueType(value) == TagValueType::WIDE_NUMBER);
	return TagValues::decimalFromWideNumber(
        valuePtr(value).getUnsignedIntUnaligned());
}

#ifdef GEODESK_PYTHON
PyObject* TagTablePtr::valueAsString(TagBits value, StringTable& strings) const
{
	if (value == 0) return strings.getStringObject(0);
	int type = valueType(value);
	if (type == TagValueType::GLOBAL_STRING)
  	{
  		return getGlobalStringObject(value, strings);
    }
	if (type == TagValueType::LOCAL_STRING)
    {
    	return getLocalStringObject(value);
    }
	if (type == TagValueType::NARROW_NUMBER)
	{
		return PyUnicode_FromFormat("%d", narrowNumber(value));
	}
	assert(type == TagValueType::WIDE_NUMBER);
	DataPtr pValue = valuePtr(value);
	uint32_t rawValue = pValue.getUnsignedInt();
	Decimal d = TagValues::decimalFromWideNumber(rawValue);
	char buf[32];
	char* p = d.format(buf);
	return PyUnicode_FromStringAndSize(buf, p-buf);
}

PyObject* TagTablePtr::valueAsObject(TagBits value, StringTable& strings) const
{
	if (value == 0)
	{
		Py_INCREF(Py_None);
		return Py_None;
	}
	int type = valueType(value);
	if (type == TagValueType::GLOBAL_STRING)
    {
		return getGlobalStringObject(value, strings);
    }
	if (type == TagValueType::LOCAL_STRING)
	{
		return getLocalStringObject(value);
    }
	if (type == TagValueType::NARROW_NUMBER)
	{
		return PyLong_FromLong(narrowNumber(value));
	}
	assert(type == TagValueType::WIDE_NUMBER);
	Decimal d = wideNumber(value);
	if(d.scale() == 0) PyLong_FromLong(d.mantissa());
	return PyFloat_FromDouble(static_cast<double>(d));
}

// TODO: Fix !!!!
PyObject* TagTablePtr::valueAsNumber(TagBits value, StringTable& strings) const
{
	if (value == 0) return PyLong_FromLong(0);
	int type = valueType(value);
	if (type == TagValueType::NARROW_NUMBER)
	{
		return PyLong_FromLong(narrowNumber(value));
	}
	if (type == TagValueType::WIDE_NUMBER)
	{
		Decimal d = wideNumber(value);
		if (d.scale() == 0) PyLong_FromLong(d.mantissa());
		return PyFloat_FromDouble(static_cast<double>(d));
	}
	if (type == TagValueType::LOCAL_STRING)
	{
		double val;
		if(!Math::parseDouble(localString(value)->toStringView(), &val)) val = 0.0;
		return PyFloat_FromDouble(val);
	}
	assert(type == TagValueType::GLOBAL_STRING);
	double val;
	if(!Math::parseDouble(globalString(value, strings)->toStringView(), &val)) val = 0.0;
	return PyFloat_FromDouble(val);
}

#endif // GEODESK_PYTHON

uint32_t TagTablePtr::count() const
{
	int count = 0;
	uint32_t tag;
	DataPtr pTable = ptr();
        // TODO: change, can just check tag (no need to incur cost of unaligned read)
	if (pTable.getUnsignedIntUnaligned() != TagValues::EMPTY_TABLE_MARKER)
	{
		DataPtr p = pTable;
		do
		{
			count++;
			tag = p.getUnsignedShort();
			p += 4 + (tag & 2);
		}
		while((tag & 0x8000) == 0);
	}
	if (hasLocalKeys())
	{
		DataPtr p = pTable - 4;
		do
		{
			count++;
			tag = p.getUnsignedShort();
			p -= 6 + (tag & 2);
		}
		while ((tag & 4) == 0);
	}
	return count;
}

