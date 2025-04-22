// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once
#include <Python.h>
#include <geodesk/feature/FeaturePtr.h>
#include <geodesk/geom/Coordinate.h>

using namespace geodesk;
class PyChangedMembers;
class PyAnonymousNode;
class PyFeature;

class PyChangedFeature : public PyObject
{
public:
	PyObject_HEAD
    int64_t id;
	uint32_t version;           // retrieved from Overpass
	uint8_t type;               // node,way,relation,member
	unsigned isDeleted : 1;
	unsigned isExplicit : 1;		 // false if a node created via coordinate, else true
	unsigned maybeHasNewParents : 1; // true if feature has been added to a way/relation
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

	enum Type { NODE,WAY,RELATION,MEMBER };

	enum Attr
	{
		LAT,
		LON,
		MEMBERS,
		NODES,
		ROLE,
		SHAPE,
		TAGS,
		X,
		Y,

		// Read-only from this point:
		COMBINE_METHOD,
		CONNECT_METHOD,
		DELETE_METHOD,
		ID,
		IS_DELETED,
		IS_NODE,
		IS_RELATION,
		IS_WAY,
		MODIFY_METHOD,
		ORIGINAL,
		OSM_TYPE,
		SPLIT_METHOD,
	};

	static constexpr Attr LAST_MUTABLE_ATTR = Y;

	static PyTypeObject TYPE;
	static PyMappingMethods MAPPING_METHODS;

	static PyChangedFeature* create(Coordinate xy);
	static PyChangedFeature* create(PyAnonymousNode* node);
	static PyChangedFeature* create(PyFeature* feature);
	static PyChangedFeature* create(PyObject* args, PyObject* kwargs);
	static void dealloc(PyChangedFeature* self);
	static PyObject* getattro(PyChangedFeature* self, PyObject *attr);
	static PyObject* getitem(PyChangedFeature* self, PyObject* key);
	static int setitem(PyChangedFeature* self, PyObject* key, PyObject* value);
	static PyObject* repr(PyChangedFeature* self);
	// static PyObject* richcompare(PyChangedFeature* self, PyObject* other, int op);
	static PyObject* str(PyChangedFeature* self);

	static PyObject* modify(PyChangedFeature* self, PyObject* args, PyObject* kwargs);
	static PyObject* delete_(PyChangedFeature* self, PyObject* args, PyObject* kwargs);

private:
	void createOrModify(PyObject* args, PyObject* kwargs, bool create);
	static PyObject* createTags(FeatureStore* store, FeaturePtr feature);
	int loadTags(bool create);
};
