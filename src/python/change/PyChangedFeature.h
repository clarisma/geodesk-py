// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once
#include <Python.h>
#include <clarisma/util/enum.h>
#include <geodesk/feature/FeaturePtr.h>
#include <geodesk/geom/FixedLonLat.h>
#include "python/Environment.h"
#include "python/util/PythonRef.h"
#include "python/change/PyChangedMembers.h"

using namespace geodesk;
namespace clarisma { class Buffer; }
class Changeset;
// class PyChangedMembers;
class PyAnonymousNode;
class PyFeature;

class PyChangedFeature : public PyObject
{
public:
	struct Flags
	{
		// first 2 bits are type
		static constexpr int DELETED = 1 << 2;
	};

	static constexpr int FLAG_COUNT = 8;

	enum Type { NODE,WAY,RELATION,MEMBER };

	enum Attr
	{
		LAT,
		LON,
		MEMBERS,
		NODES,
		ROLE,
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

	static PyChangedFeature* create(Changeset* changes, Type type);
	static PyChangedFeature* createNode(Changeset* changes, FixedLonLat lonLat);
	static PyChangedFeature* createNode(Changeset* changes, PyAnonymousNode* node);
	static PyChangedFeature* createFeature2D(Changeset* changes, PyChangedMembers* children);
	static PyChangedFeature* create(Changeset* changes, PyFeature* feature);
	static PyChangedFeature* createMember(PyChangedFeature* member, PyObject* role);
	static void dealloc(PyChangedFeature* self);
	static PyObject* getattr(PyChangedFeature* self, PyObject *attr);
	static int setattr(PyChangedFeature* self, PyObject* nameObj, PyObject* value);
	static PyObject* getitem(PyChangedFeature* self, PyObject* key);
	static int setitem(PyChangedFeature* self, PyObject* key, PyObject* value);
	static PyObject* repr(PyChangedFeature* self);
	// static PyObject* richcompare(PyChangedFeature* self, PyObject* other, int op);
	static PyObject* str(PyChangedFeature* self);

	int type() const noexcept { return idAndFlags_ & 3; }
	FeatureType featureType() const { return static_cast<FeatureType>(type()); }
	bool isMember() const { return type() == MEMBER; }
	int64_t id() const { return idAndFlags_ >> FLAG_COUNT; }
	void setId(int64_t id)
	{
		idAndFlags_ = (idAndFlags_ & ((1 << FLAG_COUNT) - 1)) | (id << FLAG_COUNT);
	}
	FixedLonLat lonLat() const noexcept
	{
		assert(type() == NODE);
		return FixedLonLat(lon_, lat_);
	}
	double lon() const
	{
		assert(type() == NODE);
		return lon_ / 1e7;
	}

	double lat() const
	{
		assert(type() == NODE);
		return lat_ / 1e7;
	}

	PyChangedMembers* children() const
	{
		assert(type() == WAY || type() == RELATION);
		return children_;
	}

	void setChildren(PyChangedMembers* children)
	{
		assert((type() == WAY && !children->containsRelationMembers()) ||
			(type() == RELATION && children->containsRelationMembers()));
		children_ = children;
	}

	PyChangedFeature* member() const
	{
		assert(type() == MEMBER);
		return member_;
	}

	PyObject* role() const
	{
		assert(type() == MEMBER);
		return role_;
	}

	PyObject* tags()
	{
		assert(type() != MEMBER);
		loadTags(true);
		// TODO: may raise, clear exception
		return tags_;
	}

	int version() const { return version_; }
	void setVersion(int version) { version_ = version; }

	bool modify(PyObject* args, PyObject* kwargs);
	static PyObject* delete_(PyChangedFeature* self, PyObject* args, PyObject* kwargs);

	void format(clarisma::Buffer& buf) const;
	static bool isTagValue(PyObject* obj);
	bool hasTags()
	{
		loadTags(false);
		// TODO: may raise, clear exception
		return tags_ != nullptr;
	}

private:
	void init(Changeset* changes_, Type type_, int64_t id_);
	void createOrModify(PyObject* args, PyObject* kwargs, bool create);
	PyObject* getAttribute(int attr);
	bool setAttribute(int attr, PyObject* value);
	bool checkAttrType(int attr, Type type) const;
	bool setMembers(PyObject* value);
	bool setNodes(PyObject* value);

	bool setTags(PyObject* value);
	static PyObject* createTags(FeatureStore* store, FeaturePtr feature);
	int loadTags(bool create);
	bool setOrRemoveTag(PyObject* key, PyObject* value);
	bool setOrRemoveTags(PyObject* dict);
	static bool isAtomicTagValue(PyObject* obj);

	Changeset* changes_;
	int64_t idAndFlags_;
	uint32_t version_;           // retrieved from Overpass
	int32_t usageCount_;
	union
	{
		struct	// if node, way or relation
		{
			PyObject* original_;
			PyObject* tags_;			// dict
			union
			{
				struct				// if node
				{
					int32_t lon_;
					int32_t lat_;
				};
				PyChangedMembers* children_;	// if way or relation
			};
		};
		struct	// if member
		{
			PyChangedFeature* member_;
			PyObject* role_;
		};
	};
};

