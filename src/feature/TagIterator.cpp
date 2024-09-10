// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#include "TagIterator.h"
#include <common/util/ShortVarString.h>


TagIterator::TagIterator(TagsRef tags, StringTable& strings) :
	tags_(tags), 
	p_(static_cast<const uint8_t*>(tags.ptr())),
	strings_(strings)
{
	// TODO: can just read the key, no need for whole tag (which
	//  is unaligned)
	if (p_.getUnsignedIntUnaligned() == TagsRef::EMPTY_TABLE_MARKER)
	{
		p_ = tags.hasLocalKeys() ? (tags.ptr() - 6) : nullptr;
	}
}

bool TagIterator::next(std::string_view& key, TagBits& value)
{
	if (!p_) return false;
	if (p_.ptr() < tags_.ptr())
	{
		TagBits tag = p_.getLongUnaligned();
		int32_t rawPointer = static_cast<int32_t>(tag >> 16);
		int32_t flags = rawPointer & 7;
		// uncommon keys are relative to the 4-byte-aligned tagtable address
		const ShortVarString* keyString = reinterpret_cast<const ShortVarString*>(
			tags_.alignedBasePtr().asBytePointer() + ((rawPointer ^ flags) >> 1));
		key = keyString->toStringView();
		value = (reinterpret_cast<TagBits>(p_.ptr() - tags_.taggedPtr() - 2) << 32)
			| ((tag & 0xffff) << 16) | flags;
		p_ = (flags & 4) ? static_cast<uint8_t*>(nullptr) : (p_.ptr() - 6 - (flags & 2));
	}
	else
	{
		// TODO: maybe just fetch key/value separately
		// to avoid unaligned read issue altogether
		uint32_t tag = p_.getUnsignedIntUnaligned();
		int keyCode = (tag & 0x7fff) >> 2;
		key = strings_.getGlobalString(keyCode)->toStringView();
		value = (reinterpret_cast<TagBits>(p_.ptr() - tags_.taggedPtr() + 2) << 32) | tag;
		int lastFlag = tag & 0x8000;
		p_ = lastFlag ? (tags_.hasLocalKeys() ? 
			(tags_.ptr() - 6) : static_cast<uint8_t*>(nullptr))
			  : (p_.ptr() + 4 + (tag & 2));
	}
	return true;
}
