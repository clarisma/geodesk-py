// Copyright (c) 2023 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include "feature/Way.h"
#include "geom/Coordinate.h"
#include "GeometryWriter.h"

class FeatureWriter : protected GeometryWriter
{
public:
	FeatureWriter(Buffer* buf) : GeometryWriter(buf) {}
	virtual ~FeatureWriter() {};
	// Attention: The base classes don't have destructors

	void pretty(bool b) { pretty_ = b; }
	void flush() { GeometryWriter::flush(); }

	virtual void writeFeature(FeatureStore* store, FeatureRef feature) = 0;
	virtual void writeHeader() {}
	virtual void writeFooter() {}

protected:
	// void writeWayCoordinates(WayRef way);

	void writeTagValue(TagsRef tags, TagBits value, StringTable& strings);


	bool pretty_ = true;
	bool firstFeature_ = true;
};
