// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once
#include <Python.h>
#include <structmember.h>

class PyChangedMembers;

class PyChangedFeature : public PyObject
{
public:
	PyObject_HEAD
    int64_t id;
	uint32_t version;           // retrieved from Overpass
	uint8_t type;               // node,way,relation,member
	unsigned isDeleted : 1;
	unsigned isExplicit : 1;		// false if a node created via coordinate, else true
	unsigned maybeHasNewParents;    // true if feature has been added to a way/relation
									// (even if later removed)
	union
	{
		struct	// if node, way or relation
		{
			PyObject* original;
			PyObject* tags;			// dict
			union
			{
				struct				// if node
				{
					int32_t x;
					int32_t y;
				};
				PyChangedMembers* nodes;	// if way
				PyChangedMembers* members;	// if relation
			};
		};
		struct	// if member
		{
			PyChangedFeature* member;
			PyObject* role;
		};
	};

	enum Attr
	{
		LAT,
		LON,
		MEMBERS,
		NODES,
		SHAPE,
		TAGS,
		X,
		Y,

		// Read-only from this point:
		COMBINE,
		CONNECT,
		DELETE,
		ID,
		IS_DELETED,
		MODIFY,
		ORIGINAL,
		SPLIT,
	};

	static PyTypeObject TYPE;
	static PyMappingMethods MAPPING_METHODS;

	static PyObject* create();
	static void dealloc(PyChangedFeature* self);
	static PyObject* getattro(PyChangedFeature* self, PyObject *attr);
	static PyObject* repr(PyChangedFeature* self);
	// static PyObject* richcompare(PyChangedFeature* self, PyObject* other, int op);
	static PyObject* str(PyChangedFeature* self);
};
