// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once
#include <Python.h>
#include <clarisma/util/enum.h>
#include <geodesk/feature/FeaturePtr.h>
#include <geodesk/geom/FixedLonLat.h>
#include "python/Environment.h"
#include "python/util/PythonRef.h"

using namespace geodesk;
namespace clarisma { class Buffer; }
class Changeset;
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

	enum Type { NODE,WAY,RELATION,MEMBER,UNASSIGNED };

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
	static PyChangedFeature* createWay(Changeset* changes, PyChangedMembers* nodes);
	static PyChangedFeature* createRelation(Changeset* changes, PyChangedMembers* members);
	static PyChangedFeature* create(Changeset* changes, PyFeature* feature);
	static PyChangedFeature* create(Changeset* changes, PyObject* args, PyObject* kwargs);
	static PyChangedFeature* createMember(PyChangedFeature* member, PyObject* role);
	static void dealloc(PyChangedFeature* self);
	static PyObject* getattr(PyChangedFeature* self, PyObject *attr);
	static int setattr(PyChangedFeature* self, PyObject* nameObj, PyObject* value);
	static PyObject* getitem(PyChangedFeature* self, PyObject* key);
	static int setitem(PyChangedFeature* self, PyObject* key, PyObject* value);
	static PyObject* repr(PyChangedFeature* self);
	// static PyObject* richcompare(PyChangedFeature* self, PyObject* other, int op);
	static PyObject* str(PyChangedFeature* self);

	int type() const noexcept { return type_; }
	FeatureType featureType() const { return static_cast<FeatureType>(type_); }
	int64_t id() { return id_; }
	void setId(int64_t id) { id_ = id; }
	FixedLonLat lonLat() const noexcept
	{
		assert(type_ == NODE);
		return FixedLonLat(lon_, lat_);
	}
	double lon() const
	{
		assert(type_ == NODE);
		return lon_ / 1e7;
	}

	double lat() const
	{
		assert(type_ == NODE);
		return lat_ / 1e7;
	}

	PyChangedMembers* nodes() const
	{
		assert(type_ == WAY);
		return nodes_;
	}

	PyChangedMembers* members() const
	{
		assert(type_ == RELATION);
		return members_;
	}

	PyChangedFeature* member() const
	{
		assert(type_ == MEMBER);
		return member_;
	}

	PyObject* role() const
	{
		assert(type_ == MEMBER);
		return role_;
	}

	PyObject* tags()
	{
		assert(type_ != MEMBER);
		loadTags(true);
		// TODO: may raise, clear exception
		return tags_;
	}

	int version() const { return version_; }
	void setVersion(int version) { version_ = version; }

	bool modify(PyObject* args, PyObject* kwargs);
	static PyObject* delete_(PyChangedFeature* self, PyObject* args, PyObject* kwargs);

	void format(clarisma::Buffer& buf);
	static bool isTagValue(PyObject* obj);
	bool hasTags()
	{
		loadTags(false);
		// TODO: may raise, clear exception
		return tags_ != nullptr;
	}

	class Parameters
	{
	public:
		Parameters(Changeset* changes, int accept) :
			changes_(changes), accept_(accept) {}

		~Parameters()
		{
			Py_XDECREF(members_);
		}
		bool parse(PyObject* args, int start, PyObject* kwargs);
		PyChangedFeature* create();

		enum
		{
			GEOMETRY = 1 << 0,
			COORDINATE = 1 << 1,
			NODES = 1 << 2,
			MEMBERS = 1 << 3
		};

	private:
		enum class Expect
		{
			ANYTHING = 0,
			LATITUDE = 1,
			TAG_VALUE = 2
		};

		struct Tag
		{
			PyObjectRef key;
			PyObjectRef value;
		};

		bool acceptTag(PyObject* key, PyObject* value);

		Changeset* changes_ = nullptr;
		int accept_ = 0;
		int received_ = 0;
		Expect expect_ = Expect::ANYTHING;
		bool receivedLon_ = false;
		bool receivedLat_ = false;
		bool receivedIndividualTags_ = false;
		bool replaceTags_ = false;
		FixedLonLat coordinate_;
		PyChangedMembers* members_ = nullptr;
		std::vector<Tag> modifiedTags_;
		std::vector<PyObjectRef> deletedKeys_;
		const GEOSGeometry* geom_ = nullptr;
	};

private:
	void init(Changeset* changes_, Type type_, int64_t id_);
	void createOrModify(PyObject* args, PyObject* kwargs, bool create);
	PyObject* getAttribute(int attr);
	bool setAttribute(int attr, PyObject* value);
	bool checkAttrType(int attr, Type type);
	bool setMembers(PyObject* value);
	bool setNodes(PyObject* value);

	bool setTags(PyObject* value);
	static PyObject* createTags(FeatureStore* store, FeaturePtr feature);
	int loadTags(bool create);
	bool setOrRemoveTag(PyObject* key, PyObject* value);
	bool setOrRemoveTags(PyObject* dict);
	static bool isAtomicTagValue(PyObject* obj);

	Changeset* changes_;
	int64_t id_;
	uint32_t version_;           // retrieved from Overpass
	uint8_t type_;               // node,way,relation,member
	Flags flags_;
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
				PyChangedMembers* nodes_;	// if way
				PyChangedMembers* members_;	// if relation
			};
		};
		struct	// if member
		{
			PyChangedFeature* member_;
			PyObject* role_;
		};
	};
};

CLARISMA_ENUM_FLAGS(PyChangedFeature::Flags)