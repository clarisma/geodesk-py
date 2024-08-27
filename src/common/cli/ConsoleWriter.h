// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include <common/util/BufferWriter.h>

class ConsoleWriter : public BufferWriter
{
public:
	ConsoleWriter();

	void flush();
	void color(int color);
	void normal();

private:
	DynamicBuffer buf_;
};
