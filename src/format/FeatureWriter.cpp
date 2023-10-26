// Copyright (c) 2023 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#include "FeatureWriter.h"
#include "feature/FeatureStore.h"

// TODO: generalize!
void FeatureWriter::writeTagValue(TagsRef tags, TagBits value, StringTable& strings)
{
	if (value & 1) // string
	{
		writeByte('\"');
		if (value & 2)
		{
			writeJsonEscapedString(tags.localString(value).toStringView());
		}
		else
		{
			writeJsonEscapedString(tags.globalString(value, strings).toStringView());
		}
		writeByte('\"');
	}
	else
	{
		if (value & 2)
		{
			formatDouble(tags.wideNumber(value));
		}
		else
		{
			formatInt(TagsRef::narrowNumber(value));
		}
	}
}

