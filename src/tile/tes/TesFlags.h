// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

namespace TesFlags
{
	// Change
	constexpr int TAGS_CHANGED = 1 << 0;
	constexpr int SHARED_TAGS = 1 << 1;
	constexpr int RELATIONS_CHANGED = 1 << 2;
	constexpr int GEOMETRY_CHANGED = 1 << 3;
	constexpr int MEMBERS_CHANGED = 1 << 4;
	constexpr int NODE_BELONGS_TO_WAY = 1 << 4;
	constexpr int NODE_IDS_CHANGED = 1 << 5;
	constexpr int BBOX_CHANGED = 1 << 5;
	constexpr int HAS_SHARED_LOCATION = 1 << 5;
	constexpr int IS_AREA = 1 << 6;
	constexpr int IS_EXCEPTION_NODE = 1 << 6;

	// Member
	constexpr int FOREIGN_MEMBER = 1 << 0;
	constexpr int DIFFERENT_ROLE = 1 << 1;
	constexpr int DIFFERENT_TILE = 1 << 2;
}
