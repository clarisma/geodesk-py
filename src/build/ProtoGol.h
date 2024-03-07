// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

namespace ProtoGol
{
	enum GroupType
	{
		LOCAL_GROUP = 1,
		EXPORTED_GROUP = 2,
		SPECIAL_GROUP = 3
	};

	enum FeatureType
	{
		NODES = 0,
		WAYS = 1,
		RELATIONS = 2
	};

	enum
	{
		LOCAL_NODES        = (FeatureType::NODES << 3) | GroupType::LOCAL_GROUP,
		LOCAL_WAYS         = (FeatureType::WAYS << 3) | GroupType::LOCAL_GROUP,
		LOCAL_RELATIONS    = (FeatureType::RELATIONS << 3) | GroupType::LOCAL_GROUP,
		EXPORTED_NODES     = (FeatureType::NODES << 3) | GroupType::EXPORTED_GROUP,
		EXPORTED_WAYS      = (FeatureType::WAYS << 3) | GroupType::EXPORTED_GROUP,
		EXPORTED_RELATIONS = (FeatureType::RELATIONS << 3) | GroupType::EXPORTED_GROUP,
		COLOCATED_NODES    = (FeatureType::NODES << 3) | GroupType::SPECIAL_GROUP,
	};
}