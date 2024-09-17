// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include "feature/TagTablePtr.h"

class TagIterator
{
public:
	TagIterator(TagTablePtr tags, StringTable& strings);

	bool next(std::string_view& key, TagBits& value);
	TagTablePtr tags() const { return tags_; }
	StringTable& strings() { return strings_; }

private:
	TagTablePtr tags_;
	DataPtr p_;
	StringTable& strings_;
};
