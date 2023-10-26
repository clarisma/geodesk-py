// Copyright (c) 2023 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include "Tags.h"

class TagIterator
{
public:
	TagIterator(TagsRef tags, StringTable& strings);

	bool next(std::string_view& key, TagBits& value);
	const TagsRef& tags() const { return tags_; }
	StringTable& strings() { return strings_; }

private:
	TagsRef tags_;
	pointer p_;
	StringTable& strings_;
};
