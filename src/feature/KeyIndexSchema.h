// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include <unordered_map>
#include <stdint.h>
#include <common/data/Span.h>

class KeyIndexSchema
{
public:
	int getCategory(int keyCode) const
	{
		auto it = keysToCategories_.find(keyCode);
		return (it != keysToCategories_.end()) ? it->second : 0;
	}

private:
	std::unordered_map<uint16_t, uint16_t> keysToCategories_;
};
