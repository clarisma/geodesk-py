// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include <common/util/DataPtr.h>

// TODO: Changes in 2.0

class BlobPtr
{
public:
	BlobPtr(const uint8_t* p) : p_(p) {}

	static uint32_t headerSize() { return 4; }

	uint32_t payloadSize() const
	{ 
		return p_.getInt() & 0x3fff'ffff;
	}

	uint32_t totalSize() const
	{
		return payloadSize() + headerSize();
	}

	DataPtr ptr() const { return p_; }

protected:
	DataPtr p_;
};
