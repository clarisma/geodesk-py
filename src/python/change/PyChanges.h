// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once
#include <Python.h>
#include <clarisma/data/HashMap.h>
#include <geodesk/feature/TypedFeatureId.h>
#include <geodesk/geom/Coordinate.h>
#include <geodesk/geom/FixedLonLat.h>

using namespace geodesk;
class PyChangedFeature;
class PyChanges;
class PyAnonymousNode;
class PyFeature;
namespace geodesk {
class FeatureStore;
}

class ChangesWeakRef
{
public:
	ChangesWeakRef(PyChanges* changes) :
		changes_(changes),
		refcount_(1) {}

	void clear() { changes_ = nullptr; }
	PyChanges* get() const { return changes_; }
	PyChanges* getOrRaise() const
	{
		if (changes_) [[likely]] return changes_;
		PyErr_SetString(PyExc_RuntimeError,
			"The associated Changes object has been deleted");
		return nullptr;
	}

	// TODO: Make threadsafe for multithreaded Python
	void addref() const { ++refcount_; }
	void release() const
	{
		if (--refcount_ == 0)
		{
			delete this;
		}
	}

private:
	PyChanges* changes_;
	mutable size_t refcount_;
};

class PyChanges : public PyObject
{
public:
	using FeaturesByCoordinate = clarisma::HashMap<Coordinate, PyChangedFeature*>;
	using FeaturesByLonLat = clarisma::HashMap<FixedLonLat, PyChangedFeature*>;
	using FeaturesByTypedId = clarisma::HashMap<TypedFeatureId, PyChangedFeature*>;
	using FeaturesVector = std::vector<PyChangedFeature*>;

	PyObject_HEAD
	FeaturesByLonLat newAnonNodes;
	FeaturesByCoordinate existingAnonNodes;
	FeaturesVector newFeatures;
	FeaturesByTypedId existingFeatures;
	PyObject* tags;           // dictionary of changeset tags
	ChangesWeakRef* weakRef;
	PyObject* outerString;
	PyObject* innerString;

	static PyTypeObject TYPE;
	static PyMethodDef METHODS[];
	static PyMappingMethods MAPPING_METHODS;

	static PyChanges* createNew(PyTypeObject* type, PyObject* args, PyObject* kwds);
	static void dealloc(PyChanges* self);
	static PyObject* getattro(PyChanges* self, PyObject *attr);
	static PyObject* getitem(PyChanges* self, PyObject* key);
	// static PyObject* richcompare(PyChanges* self, PyObject* other, int op);

	static PyObject* createFeature(PyChanges* self, PyObject* args, PyObject* kwargs);
	static PyObject* modifyFeature(PyChanges* self, PyObject* args, PyObject* kwargs);
	static PyObject* deleteFeature(PyChanges* self, PyObject* args, PyObject* kwargs);
	static PyObject* validate(PyChanges* self, PyObject* args, PyObject* kwargs);
	static PyObject* save(PyChanges* self, PyObject* args, PyObject* kwargs);

	static PyObject* str(PyChanges* self);

	PyChangedFeature* createNode(FixedLonLat lonLat);
	PyChangedFeature* createWay(PyObject* nodeList);
	PyChangedFeature* modify(FeatureStore* store, uint64_t id, Coordinate xy);
	PyChangedFeature* modify(PyFeature* feature);
	PyChangedFeature* modify(PyAnonymousNode* feature);

	ChangesWeakRef* newRef() const
	{
		weakRef->addref();
		return weakRef;
	}
};
