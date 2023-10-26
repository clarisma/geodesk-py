// Copyright (c) 2023 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#include "TagIterator.h"


TagIterator::TagIterator(TagsRef tags, StringTable& strings) :
	tags_(tags), 
	p_(tags.ptr()),
	strings_(strings)
{
	if (p_.getUnalignedUnsignedInt() == TagsRef::EMPTY_TABLE_MARKER)
	{
		p_ = tags.hasLocalKeys() ? (tags.ptr() - 6) : nullptr;
	}
}

bool TagIterator::next(std::string_view& key, TagBits& value)
{
	if (!p_) return false;
	if (p_ < tags_.ptr())
	{
		TagBits tag = p_.getUnalignedLong();
		int32_t rawPointer = static_cast<int32_t>(tag >> 16);
		int32_t flags = rawPointer & 7;
		// uncommon keys are relative to the 4-byte-aligned tagtable address
		LocalString keyString(tags_.alignedBasePtr() + ((rawPointer ^ flags) >> 1));
		key = keyString.toStringView();
		value = (static_cast<TagBits>(p_ - tags_.taggedPtr() - 2) << 32) 
			| ((tag & 0xffff) << 16) | flags;
		p_ = (flags & 4) ? nullptr : (p_ - 6 - (flags & 2));
	}
	else
	{
		// TODO: maybe just fetch key/value separately
		// to avoid unaligned read issue altogether
		uint32_t tag = p_.getUnalignedUnsignedInt();
		int keyCode = (tag & 0x7fff) >> 2;
		key = strings_.getGlobalString(keyCode).toStringView();
		value = (static_cast<TagBits>(p_ - tags_.taggedPtr() + 2) << 32) | tag;
		int lastFlag = tag & 0x8000;
		p_ = lastFlag ? (tags_.hasLocalKeys() ? 
			(tags_.ptr() - 6) : nullptr) 
			  : (p_ + 4 + (tag & 2));
	}
	return true;
}
