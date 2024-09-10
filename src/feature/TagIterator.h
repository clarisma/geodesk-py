// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include "Tags.h"
#include <common/util/DataPtr.h>

class TagIterator
{
public:
	TagIterator(TagsRef tags, StringTable& strings);

	bool next(std::string_view& key, TagBits& value);
	const TagsRef& tags() const { return tags_; }
	StringTable& strings() { return strings_; }

private:
	TagsRef tags_;
	DataPtr p_;
	StringTable& strings_;
};
