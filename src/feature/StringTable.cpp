// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#include "StringTable.h"

#include <api/StringValue.h>
#include <common/util/Bits.h>
#include <common/util/PbfDecoder.h>
#include <common/util/Strings.h>
#include <python/util/util.h>

// TODO: We have the problem that strign table index 0 is a valid entry (empty string)
// However, during lookup, when we encounter a next-slot value of 0, we don't know
// whether this means the next entry is #0, or we're at the end of the list
// We can mitigate this by always indexing "" last, so it is always indexed at the
// head of the chain (i.e. no entry will ever have it as the next entry)
// We are already indexing the strings from highest to lowest, to ensure that the 
// most frequently indexed strings are towards the heads of the collision chain
// --> No, does not work, initial lookup would still be 0 for "", need to 
//     use 1-based indexes
// Or, could simply test for 0-length string and return code 0; the hash function
// will be more efficient if we can guarantee non-zero length string

// TODO: To support Python 3.13, we'll have to stop using _Py_HashBytes;
// instead, we use Strings::hash. This means we no longer can call PyObject_Hash()
// when looking up global-key codes using a Python string. PyObject_Hash() will return
// the cached hash, instead of re-calculating it each time. But our hash implementation
// is very fast (and key strings are short), and will likely be inlined, so it may 
// end up being faster than calling the Python.dll function (which also dispatches 
// virtually again via the tp_hash slot of the PyUnicode object)
// Alternative: eagerly load all strings as Python string objects, and put them in
// a separate lookup table (we still need to support lookup via std::string_view
// for our internal needs, and we can't use the same table because we no longer have 
// access to Python's hash function (unless we first turn the std::string_view into
// a temporary Python string)

StringTable::StringTable() :
	arena_(nullptr)
{
	// TODO: clear all other members?
}

void StringTable::create(const uint8_t* pStrings)
{
	stringBase_ = pStrings;
	PbfDecoder data(pStrings);
	stringCount_ = data.readVarint32() + 1;
	// currently, "" is not stored in string table
	unsigned long leadingZeroes;

	// Round up string count to next-highest power-of-2, then double it
	// to get a decent hashtable size
	leadingZeroes = Bits::countLeadingZerosInNonZero32(stringCount_);
	uint32_t bucketCount = 1U << (32 - leadingZeroes);
	lookupMask_ = bucketCount - 1;

	#ifdef GEODESK_PYTHON
	int stringObjectTableSize = stringCount_ * sizeof(PyObject*);
	#else
	int stringObjectTableSize = 0;
	#endif
	int entryTableSize = stringCount_ * sizeof(Entry*);
	int arenaSize =
		stringObjectTableSize +
		entryTableSize +
		bucketCount * sizeof(uint16_t);
	arena_ = new uint8_t[arenaSize];
	#ifdef GEODESK_PYTHON
	stringObjects_ = reinterpret_cast<PyObject**>(arena_);
	#endif
	entries_ = reinterpret_cast<Entry*>(arena_ + stringObjectTableSize);
	buckets_ = reinterpret_cast<uint16_t*>(arena_
		+ stringObjectTableSize + entryTableSize);

	// clear the entire arena
	std::memset(arena_, 0, arenaSize);

	for (uint32_t i = 1; i < stringCount_; i++)
	{
		entries_[i].relPointer = static_cast<uint32_t>(data.pointer() - pStrings);
		// next has already been initialized with 0
		uint32_t len = data.readVarint32();
		data.skip(len);
	}

	// We'll index strings starting with highest numbers first,
	// so more commonly used strings will be placed towards the head
	// of the collision list

	for (int i = stringCount_ - 1; i > 0; i--)
	{
		const ShortVarString* str = getGlobalString(i);
		size_t hash = Strings::hashNonEmpty(str->data(), str->length());
		int bucket = hash & lookupMask_;
		uint16_t oldEntry = buckets_[bucket];
		if (oldEntry) entries_[i].next = oldEntry;
		buckets_[bucket] = i;
	}

	#ifdef GEODESK_PYTHON
	// TODO: This may change if we store "" in the GOL's global strings
	// PyObject* emptyStr = PyUnicode_NewEmptyUnicodeObject();
	stringObjects_[0] = PyUnicode_InternFromString("");
	#endif
}


StringTable::~StringTable()
{
	if (arena_)
	{
		#ifdef GEODESK_PYTHON
		for (uint32_t i = 0; i < stringCount_; i++)
		{
			PyObject* strObj = stringObjects_[i];
			if (strObj) Py_DECREF(strObj);
		}
		#endif
		delete[] arena_;
	}
}


bool StringTable::isValidCode(int code)
{
	return code >= 0 && code < static_cast<int>(stringCount_);
}

// TODO: toStringObject() may return NULL in case of failure!
#ifdef GEODESK_PYTHON
PyObject* StringTable::getStringObject(int code)
{
	assert(code >= 0 && code < stringCount_);
	PyObject* strObj = stringObjects_[code];
	if (!strObj)
	{
		geodesk::StringValue str(stringBase_ + entries_[code].relPointer);
		strObj = Python::toStringObject(str);
		assert(strObj);
		stringObjects_[code] = strObj;
	}
	Py_INCREF(strObj);
	return strObj;
}
#endif

int StringTable::getCode(size_t hash, const char* str, size_t len) const
{
	int bucket = hash & lookupMask_;
	uint16_t code = buckets_[bucket];
	while (code)
	{
		const Entry& entry = entries_[code];
		const ShortVarString* candidate =
			reinterpret_cast<const ShortVarString*>(stringBase_ + entry.relPointer);
		if (candidate->equals(str, len)) return code;
		code = entry.next;
	}
	return -1;
}

int StringTable::getCode(const char* str, size_t len) const
{
	if (len == 0) return 0;		
		// "" is always global code #0; by doing this check upfront,
		// we avoid the problem of having entry 0 in any of the hashtable
		// chains, as we use 0 as the end-of-chain marker
		// We can then use the slightly faster hash function for non-empty strings
	size_t hash = Strings::hashNonEmpty(str, len);
	return getCode(hash, str, len);
}

