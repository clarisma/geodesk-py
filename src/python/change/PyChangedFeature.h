// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once
#include <Python.h>
#include <clarisma/util/enum.h>
#include <geodesk/feature/FeaturePtr.h>
#include <geodesk/geom/FixedLonLat.h>
#include "python/Environment.h"

using namespace geodesk;
namespace clarisma { class Buffer; }
class ChangesWeakRef;
class PyChanges;
class PyChangedMembers;
class PyAnonymousNode;
class PyFeature;

class PyChangedFeature : public PyObject
{
public:
	enum class Flags : uint16_t
	{
		DELETED = 1 << 0,
		NODE_LOCATION_CHANGED = 1 << 1,
	};

	ChangesWeakRef* changes;
    int64_t id;
	uint32_t version;           // retrieved from Overpass
	uint8_t type;               // node,way,relation,member
	Flags flags;
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
					int32_t lon;
					int32_t lat;
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

	enum Type { NODE,WAY,RELATION,MEMBER,UNASSIGNED };

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

	enum AttrGroup
	{
		GROUP_X = 1 << 0,			// lon, x, lonlat, xy
		GROUP_Y = 1 << 1,			// lat, y, lonlat, xy
		GROUP_SHAPE = 1 << 2,		// shape, nodes, members
		GROUP_TAGS_ALL = 1 << 3,	// tags
		GROUP_TAGS_INDIVIDUAL = 1 << 4,	// individual tags
	};

	static constexpr Attr LAST_MUTABLE_ATTR = Y;

	static PyTypeObject TYPE;
	static PyMappingMethods MAPPING_METHODS;

	static PyChangedFeature* create(PyChanges* changes, Type type);
	static PyChangedFeature* create(PyChanges* changes, FixedLonLat lonLat);
	static PyChangedFeature* create(PyChanges* changes, PyAnonymousNode* node);
	static PyChangedFeature* create(PyChanges* changes, PyFeature* feature);
	static PyChangedFeature* create(PyChanges* changes, PyObject* args, PyObject* kwargs);
	static PyChangedFeature* createMember(PyChangedFeature* member, PyObject* role);
	static void dealloc(PyChangedFeature* self);
	static PyObject* getattr(PyChangedFeature* self, PyObject *attr);
	static int setattr(PyChangedFeature* self, PyObject* nameObj, PyObject* value);
	static PyObject* getitem(PyChangedFeature* self, PyObject* key);
	static int setitem(PyChangedFeature* self, PyObject* key, PyObject* value);
	static PyObject* repr(PyChangedFeature* self);
	// static PyObject* richcompare(PyChangedFeature* self, PyObject* other, int op);
	static PyObject* str(PyChangedFeature* self);

	bool modify(PyObject* args, PyObject* kwargs);
	static PyObject* delete_(PyChangedFeature* self, PyObject* args, PyObject* kwargs);

	void format(clarisma::Buffer& buf);
	static bool isTagValue(PyObject* obj);

private:
	void init(PyChanges* changes_, Type type_, int64_t id_);
	void createOrModify(PyObject* args, PyObject* kwargs, bool create);
	int setProperty(int attr, PyObject* value);
	void wrongAttrForType(int attr, Type only);
	bool applyAttr(int attr, Type only);
	bool setMembers(PyObject* value);
	bool setNodes(PyObject* value);

	bool setTags(PyObject* value);
	static PyObject* createTags(FeatureStore* store, FeaturePtr feature);
	int loadTags(bool create);
	bool setOrRemoveTag(PyObject* key, PyObject* value);
	bool setOrRemoveTags(PyObject* dict);
	static bool isAtomicTagValue(PyObject* obj);

	bool setShape(PyObject* value);

	class Builder
	{
	public:
		Builder();
		~Builder()
		{
			Py_XDECREF(list_);
			// only the list is owned, all others are borrowed
		}
		bool build(PyChangedFeature* feature, PyObject* args, PyObject* kwargs);

	private:
		bool pushNode();
		int tryCreateFeature(PyObject* value, PyChangedFeature** feature);

		PyChanges* changes_ = nullptr;
		PyChangedFeature* feature_ = nullptr;
		PyObject* list_ = nullptr;		// This is owned by Builder
		PyObject* key_ = nullptr;
		double xOrLon_ = 0;
		double yOrLat_ = 0;
		int seenArgs_ = 0;
		int coordValueCount_ = 0;		// 0, 1 (read x), 2 (read x/y)
		bool isMemberList_ = false;
	};
};

CLARISMA_ENUM_FLAGS(PyChangedFeature::Flags)