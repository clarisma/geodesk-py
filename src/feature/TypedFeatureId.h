// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include "types.h"
#include <common/text/Format.h>

class TypedFeatureId
{
public:
	TypedFeatureId(uint64_t typedId) noexcept : typedId_(typedId) {}

	static TypedFeatureId ofTypeAndId(int type, uint64_t id) noexcept
	{
		assert(type >= 0 && type <= 2);
		return TypedFeatureId((id << 2) | type);
	}

	operator uint64_t() const noexcept
	{
		return typedId_;
	}

	uint64_t id() const noexcept { return typedId_ >> 2; }
	int type() const noexcept { return typedId_ & 3; }
	bool isNode() const { return type() == FeatureType::NODE; }

	// TODO: will change in 2.0
	uint64_t asIdBits() const noexcept
	{
		uint64_t hi = (typedId_ >> 34) << 8;
		uint64_t lo = (typedId_ >> 2) << 32;
		return hi | lo | (type() << 3);
	}
	 
	void format(char* buf) const
	{
		const char* s;
		int len;
		switch (type())
		{
		case 0:
			s = "node";
			len = 4;
			break;
		case 1:
			s = "way";
			len = 3;
			break;
		case 2:
			s = "relation";
			len = 8;
			break;
		case 3:
			s = "invalid";
			len = 7;
			break;
		}
		memcpy(buf, s, len);
		buf[len] = '/';
		Format::integer(buf + len + 1, id());
	}

	std::string toString() const
	{
		char buf[32];
		format(buf);
		return std::string(buf);
	}

private:
	uint64_t typedId_;
};
