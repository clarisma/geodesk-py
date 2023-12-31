// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#include "Feature.h"
#include "FeatureStore.h"
#include "types.h"
#include <common/util/StringBuilder.h>


const char* FeatureRef::typeName() const
{
	uint32_t flags = ptr_.getUnsignedInt();
	int type = (flags >> 3) & 3;
	if (type == 0) return "node";
	if (type == 1) return "way";
	if (type == 2) return "relation";
	return "invalid";
}


std::string FeatureRef::toString() const
{
	char buf[50];
	sprintf(buf, "%s/%llu", typeName(), id());
	return std::string(buf);
}


