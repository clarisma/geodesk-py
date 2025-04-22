// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#include "PyChangedFeature.h"

#include <geodesk/feature/NodePtr.h>
#include <geodesk/feature/TagIterator.h>

#include "python/feature/PyFeature.h"

PyChangedFeature* PyChangedFeature::create(Coordinate xy)
{
	PyChangedFeature* self = (PyChangedFeature*)TYPE.tp_alloc(&TYPE, 0);
	if (self)
	{
		self->id = 0;
		self->version = 0;
		self->type = NODE;
		self->original = nullptr;
		self->tags = nullptr;
		self->x = xy.x;
		self->y = xy.y;
	}
	return self;
}

PyChangedFeature* PyChangedFeature::create(PyAnonymousNode* node)
{
	PyChangedFeature* self = (PyChangedFeature*)TYPE.tp_alloc(&TYPE, 0);
	if (self)
	{
		self->id = 0;			// TODO: in v2, anon nodes can have id
		self->version = 0;
		self->type = NODE;
		self->isExplicit = true;
		self->original = node;
		self->tags = nullptr;
		self->x = node->x_;
		self->y = node->y_;
	}
	return self;
}

PyChangedFeature* PyChangedFeature::create(PyFeature* feature)
{
	PyChangedFeature* self = (PyChangedFeature*)TYPE.tp_alloc(&TYPE, 0);
	if (self)
	{
		self->id = 0;			// TODO: in v2, anon nodes can have id
		self->version = 0;
		self->type = feature->feature.typeCode();
		self->isExplicit = true;
		self->original = feature;
		self->tags = nullptr;	// tags are lazy
		switch (self->type)
		{
		case NODE:
			NodePtr node(feature->feature);
			self->x = node.x();
			self->y = node.y();
			break;
		case WAY:
			self->nodes = nullptr;	// lazy
			break;
		case RELATION:
			self->members = nullptr;	// lazy
			break;
		default:
			assert(false);
		}
	}
	return self;
}


void PyChangedFeature::dealloc(PyChangedFeature* self)
{
	if (self->type == MEMBER)
	{
		Py_XDECREF(self->member);
		Py_XDECREF(self->role);
	}
	else
	{
		Py_XDECREF(self->original);
		Py_XDECREF(self->tags);
		if (self->type == WAY)
		{
			Py_XDECREF(self->nodes);
		}
		else if (self->type == RELATION)
		{
			Py_XDECREF(self->members);
		}
	}
}

PyObject* PyChangedFeature::getattro(PyChangedFeature* self, PyObject *attr)
{
	// TODO
	Py_RETURN_NONE;
}


PyObject* PyChangedFeature::repr(PyChangedFeature* self)
{
	// TODO
	Py_RETURN_NONE;
}

/*
PyObject* PyChangedFeature::richcompare(PyChangedFeature* self, PyObject* other, int op)
{
	// TODO
	Py_RETURN_NONE;
}
*/

PyObject* PyChangedFeature::str(PyChangedFeature* self)
{
	// TODO
	Py_RETURN_NONE;
}

PyObject* PyChangedFeature::createTags(FeatureStore* store, FeaturePtr feature)
{
	PyObject* dict = PyDict_New();
	if (!dict) return nullptr;

	TagIterator iter(feature.tags(), store->strings());
	for (;;)
	{
		auto [keyStr, tagBits] = iter.next();
		if (keyStr == nullptr) break;

		PyObject* key = Python::toStringObject(keyStr->data(), keyStr->length());
		if (!key)
		{
			Py_DECREF(dict);
			return nullptr;
		}

		PyObject* value = iter.tags().valueAsObject(tagBits, store->strings());
		if (!value)
		{
			Py_DECREF(key);
			Py_DECREF(dict);
			return nullptr;
		}

		if (PyDict_SetItem(dict, key, value) < 0)
		{
			Py_DECREF(key);
			Py_DECREF(value);
			Py_DECREF(dict);
			return nullptr;
		}

		Py_DECREF(key);
		Py_DECREF(value);
	}
	return dict;
}


int PyChangedFeature::loadTags(bool create)
{
	assert(type != MEMBER);
	if (tags) return 1;
	if (!original || Py_TYPE(original) == &PyAnonymousNode::TYPE)
	{
		if (!create) return 0;
		tags = PyDict_New();
	}
	else
	{
		assert(Py_TYPE(original) == &PyFeature::TYPE);
		PyFeature* feature = (PyFeature*)original;
		tags = createTags(feature->store, feature->feature);
	}
	return tags ? 1 : -1;
}

PyObject* PyChangedFeature::getitem(PyChangedFeature* self, PyObject* key)
{
	if (self->type == MEMBER)	[[unlikely]]
	{
		return getitem(self->member, key);	// delegate to member
	}
	int res = self->loadTags(false);
	if (res <= 0)
	{
		if (res < 0) return nullptr;
		Py_RETURN_NONE;
	}
	PyObject* value = PyDict_GetItem(self->tags, key);
	if (!value)
	{
		Py_RETURN_NONE;
	}
	return Python::newRef(value);
}

int PyChangedFeature::setitem(PyChangedFeature* self, PyObject* key, PyObject* value)
{
	if (self->type == MEMBER)	[[unlikely]]
	{
		return setitem(self->member, key, value);	// delegate to member
	}
	int res = self->loadTags(true);
	if (res < 0) return -1;
	assert(res > 0);

	// value == nullptr => deletion
	// TODO: setting to empty string should also delete
	if (value == nullptr || value == Py_None)
	{
		return PyObject_DelItem(self->tags, key);
	}
	return PyObject_SetItem(self->tags, key, value);
}


PyMappingMethods PyChangedFeature::MAPPING_METHODS =
{
	nullptr,         // mp_length (optional)
	(binaryfunc)getitem,         // mp_subscript
	(objobjargproc)setitem       // mp_ass_subscript
};

PyTypeObject PyChangedFeature::TYPE =
{
	.tp_name = "geodesk.ChangedFeature",
	.tp_basicsize = sizeof(PyChangedFeature),
	.tp_dealloc = (destructor)dealloc,
	.tp_repr = (reprfunc)repr,
	.tp_as_mapping = &MAPPING_METHODS,
	.tp_str = (reprfunc)str,
	.tp_getattro = (getattrofunc)getattro,
	.tp_flags = Py_TPFLAGS_DEFAULT, // | Py_TPFLAGS_DISALLOW_INSTANTIATION,
	.tp_doc = "ChangedFeature objects",
	// .tp_richcompare = (richcmpfunc)richcompare,
};
