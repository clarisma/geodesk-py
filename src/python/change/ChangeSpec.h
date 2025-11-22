// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once
#include <geos_c.h>
#include <geodesk/geom/FixedLonLat.h>
#include "python/util/PythonRef.h"
#include "ChangedTags.h"

using namespace geodesk;
class Changeset;
class PyChangedFeature;

class ChangeSpec
{
public:
	ChangeSpec(Changeset* changes, int accept) :
		changes_(changes), accept_(accept)
	{
		assert(changes);
	}

	bool parse(PyObject* args, int start, PyObject* kwargs);
	PyChangedFeature* create();
	bool modify(PyChangedFeature* feature) const;

	enum
	{
		GEOMETRY = 1 << 0,
		COORDINATE = 1 << 1,
		NODES = 1 << 2,
		MEMBERS = 1 << 3
	};

private:
	struct Tag
	{
		PyObject* key;
		PyObject* value;
	};

	static const char* shapeTypeName(int shapeType);
	bool acceptShapeType(int shapeType);
	bool acceptSequenceArg(PyObject* seq);
	bool acceptCoordinate(PyObject* first, PyObject* second);
	bool acceptChildSequence(PyObject* seq);
	PyChangedFeature* acceptMember(PyObject* obj);
	bool changeTags(PyChangedFeature* feature) const;

	static void errorExpectedTag();

	Changeset* changes_ = nullptr;
	int accept_ = 0;
	int received_ = 0;
	FixedLonLat coordinate_;
	PythonRef<PyObject> children_;
	ChangedTags tags_;
	const GEOSGeometry* geom_ = nullptr;
};
