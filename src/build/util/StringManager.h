// Copyright (c) 2023 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

class BuildSettings;
class StringStatistics;

class StringManager
{
public:
	StringManager();

	void build(const BuildSettings& setings, const StringStatistics& strings);

private:
	struct Entry
	{
		uint32_t next;
		uint32_t keyCode;
		uint32_t valueCode;
		ShortVarString string;
	};
};
