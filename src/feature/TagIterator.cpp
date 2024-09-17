// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#include "feature/TagIterator.h"
#include <common/util/ShortVarString.h>


TagIterator::TagIterator(TagTablePtr tags, StringTable& strings) :
	tags_(tags), 
	p_(tags.ptr()),
	strings_(strings)
{
	// TODO: can just read the key, no need for whole tag (which
	//  is unaligned)
	if (p_.getUnsignedIntUnaligned() == TagValues::EMPTY_TABLE_MARKER)
	{
		p_ = tags.hasLocalKeys() ? (tags.ptr() - 6) : DataPtr();
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
		// local keys are relative to the 4-byte-aligned tagtable address
		const ShortVarString* keyString = reinterpret_cast<const ShortVarString*>(
			tags_.alignedBasePtr().ptr() + ((rawPointer ^ flags) >> 1));
		key = keyString->toStringView();
		value = (static_cast<TagBits>(tags_.pointerOffset(p_) - 2) << 32) | ((tag & 0xffff) << 16) | flags;
		p_ = (flags & 4) ? DataPtr() : (p_ - 6 - (flags & 2));
	}
	else
	{
		// TODO: maybe just fetch key/value separately
		//  to avoid unaligned read issue altogether
		uint32_t tag = p_.getUnsignedIntUnaligned();
		int keyCode = (tag & 0x7fff) >> 2;
		key = strings_.getGlobalString(keyCode)->toStringView();
		/*
		printf("p_ = %p\n", p_.ptr());
		printf("tags_ (raw) = %p\n", tags_.taggedPtr().rawPtr());
		printf("Pointer offset = %d\n", tags_.pointerOffset(p_));
		fflush(stdout);
		*/
		value = (static_cast<TagBits>(tags_.pointerOffset(p_) + 2) << 32) | tag;
		int lastFlag = tag & 0x8000;
		// If we're at the end:
		//   Move pointer to first local tag (if any), or null
		// else:
		//   Move pointer to the next tag (bit 1 indicates whether value is
		//   2 or 4 bytes wide)
		p_ = lastFlag ? (tags_.hasLocalKeys() ? (tags_.ptr() - 6) : DataPtr()) : (p_ + 4 + (tag & 2));
	}
	return true;
}
