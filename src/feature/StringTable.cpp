// Copyright (c) 2023 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#include "StringTable.h"

#include <common/util/Bits.h>
#include <common/util/PbfDecoder.h>

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
	leadingZeroes = Bits::countLeadingZerosInNonZero(stringCount_);
	uint32_t bucketCount = 1U << (32 - leadingZeroes);
	lookupMask_ = bucketCount - 1;

	int stringObjectTableSize = stringCount_ * sizeof(PyObject*);
	int entryTableSize = stringCount_ * sizeof(Entry*);
	int arenaSize =
		stringObjectTableSize +
		entryTableSize +
		bucketCount * sizeof(uint16_t);
	arena_ = new uint8_t[arenaSize];
	stringObjects_ = reinterpret_cast<PyObject**>(arena_);
	entries_ = reinterpret_cast<Entry*>(arena_ + stringObjectTableSize);
	buckets_ = reinterpret_cast<uint16_t*>(arena_
		+ stringObjectTableSize + entryTableSize);

	// clear the entire arena
	std::memset(arena_, 0, arenaSize);

	for (uint32_t i = 1; i < stringCount_; i++)
	{
		entries_[i].relPointer = data.pointer() - pStrings;
		// next has already been initialzied with 0
		uint32_t len = data.readVarint32();
		data.skip(len);
	}
	for (int i = stringCount_ - 1; i > 0; i--)
	{
		GlobalString str(stringBase_ + entries_[i].relPointer);
		Py_hash_t hash = _Py_HashBytes(str.data(), str.length());
		int bucket = hash & lookupMask_;
		uint16_t oldEntry = buckets_[bucket];
		if (oldEntry) entries_[i].next = oldEntry;
		buckets_[bucket] = i;
	}

	// TODO: This may change if we store "" in the GOL's global strings
	// PyObject* emptyStr = PyUnicode_NewEmptyUnicodeObject();
	stringObjects_[0] = PyUnicode_InternFromString("");
}


StringTable::~StringTable()
{
	if (arena_)
	{
		for (uint32_t i = 0; i < stringCount_; i++)
		{
			PyObject* strObj = stringObjects_[i];
			if (strObj) Py_DECREF(strObj);
		}
		delete[] arena_;
	}
}


GlobalString StringTable::getGlobalString(int code)
{
	assert(code >= 0 && code < stringCount_);
	return GlobalString(stringBase_ + entries_[code].relPointer);
}

bool StringTable::isValidCode(int code)
{
	return code >= 0 && code < stringCount_;
}


PyObject* StringTable::getStringObject(int code)
{
	assert(code >= 0 && code < stringCount_);
	PyObject* strObj = stringObjects_[code];
	if (!strObj)
	{
		GlobalString str(stringBase_ + entries_[code].relPointer);
		strObj = str.toStringObject();
		stringObjects_[code] = strObj;
	}
	Py_INCREF(strObj);
	return strObj;
}

int StringTable::getCode(Py_hash_t hash, const char* str, int len) const
{
	int bucket = hash & lookupMask_;
	uint16_t code = buckets_[bucket];
	while (code)
	{
		const Entry& entry = entries_[code];
		GlobalString candidate(stringBase_ + entry.relPointer);
		if (candidate.equals(str, len)) return code;
		code = entry.next;
	}
	return -1;
}

int StringTable::getCode(const char* str, int len) const
{
	Py_hash_t hash = _Py_HashBytes(str, len);
	return getCode(hash, str, len);
}

int StringTable::getCode(PyObject* strObj) const
{
	const char* str;
	Py_ssize_t len;
	str = PyUnicode_AsUTF8AndSize(strObj, &len);
	return getCode(PyObject_Hash(strObj), str, static_cast<int>(len));
	// TODO: Could get hash directly, but this is not in the public API
}

