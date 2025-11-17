// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#include "Changeset.h"
#include "PyChangedFeature.h"
#include "PyChangedMembers.h"
#include "python/feature/PyFeature.h"
#include "python/util/util.h"

using namespace clarisma;

Changeset::Changeset(PyObject* tags) :
	refcount_(1)
{
	tags_.reset((tags));
	outerString_.reset(PyUnicode_FromString("outer"));
	innerString_.reset(PyUnicode_FromString("inner"));
}

void Changeset::addCreated(PyChangedFeature* f)
{
	if (live_) [[likely]]
	{
		int type = f->type;
		created_[type].emplace_back(PyFeatureRef::makeNew(f));
		f->setId(-created_[type].size());
	}
}

PyChangedFeature* Changeset::createNode(FixedLonLat lonLat)
{
	PyChangedFeature* node;
	auto it = createdAnonNodes_.find(lonLat);
	if (it != createdAnonNodes_.end())
	{
		node = it->second.get();
		if (!node->hasTags() && node->lonLat() == lonLat)
		{
			return Python::newRef();
		}
	}
	node = PyChangedFeature::create(this, lonLat);
	if (!node) return nullptr;
	if (live_)	[[likely]]
	{
		createdAnonNodes_[lonLat] = PyFeatureRef::makeNew(node);
		addCreated(node);
	}
	return node;
}

PyChangedFeature* Changeset::createWay(PyObject* nodeList)	// steals ref
{
	PyChangedMembers* nodes = PyChangedMembers::create(this, nodeList, false);
		// steals ref to nodeList even if it fails
	if (!nodes) return nullptr;
	PyChangedFeature* way =  PyChangedFeature::create(this, PyChangedFeature::WAY);
	if (!way)
	{
		Py_DECREF(nodes);
		return nullptr;
	}
	way->nodes = nodes;
	addCreated(way);
	return way;
}

PyChangedFeature* Changeset::createRelation(PyObject* memberList)
{
	PyChangedMembers* members = PyChangedMembers::create(this, memberList, true);
	// steals ref to nodeList even if it fails
	if (!members) return nullptr;
	PyChangedFeature* rel =  PyChangedFeature::create(this, PyChangedFeature::RELATION);
	if (!rel)
	{
		Py_DECREF(members);
		return nullptr;
	}
	rel->members = members;
	addCreated(rel);
	return rel;
}

PyChangedFeature* Changeset::modify(FeatureStore* store, uint64_t id, Coordinate xy)
{
	auto it = existingAnonNodes_.find(xy);
	if (it != existingAnonNodes_.end())
	{
		return Python::newRef(it->second.get());
	}
	PyChangedFeature* node = PyChangedFeature::create(this,
		PyAnonymousNode::create(store, id, xy.x, xy.y));
	if (!node) return nullptr;
	if (live_) [[likely]]
	{
		existingAnonNodes_[xy] = PyFeatureRef(node);
	}
	return node;
}

PyChangedFeature* Changeset::modify(PyAnonymousNode* node)
{
	Coordinate xy(node->x_, node->y_);
	auto it = existingAnonNodes_.find(xy);
	if (it != existingAnonNodes_.end())
	{
		return Python::newRef(it->second.get());
	}
	PyChangedFeature* changed = PyChangedFeature::create(this, node);
	if (!changed) return nullptr;
	if (live_) [[likely]]
	{
		existingAnonNodes_[xy] = PyFeatureRef(changed);
	}
	return changed;
}

PyChangedFeature* PyChanges::modify(PyFeature* feature)
{
	uint64_t id = feature->feature.id();
	auto& features = model_.existing[feature->feature.typeCode()];
	auto it = features.find(id);
	if (it != features.end())
	{
		return Python::newRef(it->second.get());
	}
	PyChangedFeature* changed = PyChangedFeature::create(this, feature);
	if (!changed) return nullptr;
	features[id] = PyFeatureRef(changed);
	return Python::newRef(changed);
}

PyObject* PyChanges::createFeature(PyChanges* self, PyObject* args, PyObject* kwargs)
{
	PyChangedFeature::Parameters params(self,
		PyChangedFeature::Parameters::GEOMETRY |
		PyChangedFeature::Parameters::COORDINATE |
		PyChangedFeature::Parameters::NODES |
		PyChangedFeature::Parameters::MEMBERS);
	if (!params.parse(args, 0, kwargs)) return nullptr;

	Py_RETURN_NONE;
}

