// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#include "PyTile.h"
#include "python/geom/PyBox.h"

#include "PyTile_lookup.cxx"
#include "python/feature/PyFeature.h"

PyTile* PyTile::create(FeatureStore* store, Tile tile, Tip tip)
{
	PyTile* self = (PyTile*)TYPE.tp_alloc(&TYPE, 0);
	if (self)
	{
		store->addref();
		self->store = store;
		self->tile_ = tile;
		self->tip_ = tip;
	}
	return self;
}

void PyTile::dealloc(PyTile* self)
{
	self->store->release();
	Py_TYPE(self)->tp_free(self);
}

PyObject* PyTile::getattro(PyTile* self, PyObject* nameObj)
{
	Py_ssize_t len;
	const char* name = PyUnicode_AsUTF8AndSize(nameObj, &len);
	if (!name) return NULL;

	PyTileAttribute* attr = PyTile_AttrHash::lookup(name, len);
	if (attr)
	{
		Python::AttrRef ref = attr->attr;
		assert(!ref.isMethod());
		return ref.getter()(self);
	}
	return PyObject_GenericGetAttr(self, nameObj);
}

Py_hash_t PyTile::hash(PyTile* self)
{
	return static_cast<uint32_t>(self->tile_);
}

/*
PyObject* PyTile::iter(PyTile* self)
{
	// TODO
	Py_RETURN_NONE;
}

PyObject* PyTile::next(PyTile* self)
{
	// TODO
	Py_RETURN_NONE;
}
*/

PyObject* PyTile::repr(PyTile* self)
{
	return str(self);	// TODO
}


PyObject* PyTile::richcompare(PyTile* self, PyObject* other, int op)
{
	PyObject* res;
	if (Py_TYPE(other) == &TYPE)
	{
		PyTile* tile = (PyTile*)other;
		switch (op)
		{
		case Py_EQ:
			res = (self->store == tile->store && self->tip == tile->tip) ?
				Py_True : Py_False;
			break;
		case Py_NE:
			res = (self->store != tile->store || self->tip != tile->tip) ?
				Py_True : Py_False;
			break;
		default:
			res = Py_NotImplemented;
			break;
		}
	}
	else
	{
		switch (op)
		{
		case Py_EQ:
			res = Py_False;
			break;
		case Py_NE:
			res = Py_True;
			break;
		default:
			res = Py_NotImplemented;
			break;
		}
	}
	return Python::newRef(res);
}


PyObject* PyTile::str(PyTile* self)
{
	Tile tile = self->tile_;
	return PyUnicode_FromFormat("%d/%d/%d", tile.zoom(), tile.column(), tile.row());
}

PyObject* PyTile::bounds(PyTile* self)
{
	return PyBox::create(self->tile_.bounds());
}

PyObject* PyTile::children(PyTile* self)
{
	Py_RETURN_NONE;	// TODO
}

PyObject* PyTile::column(PyTile* self)
{
	return PyLong_FromLong(self->tile_.column());
}

PyObject* PyTile::exports(PyTile* self)
{
	if (self->store)
	{
		TilePtr pTile = self->store->fetchTile(self->tip_);
		if (pTile)
		{
			uint32_t count = 0;
			ExportTablePtr pExports = pTile.exports();
			if (pExports) count = pExports.count();
			PyObject* list = PyList_New(count);
			if (!list) return nullptr;
			for (int i = 0; i < count; ++i)
			{
				PyObject* item;
				// Tex is 1-based
				FeaturePtr pFeature = pExports.featureAt(i + 1);
				if (pFeature.isNull())
				{
					item = Python::newRef(Py_None);
				}
				else
				{
					item = PyFeature::create(self->store, pFeature, Py_None);
					if (!item)
					{
						Py_DecRef(list);
						return nullptr;
					}
				}
				PyList_SET_ITEM(list, i, item);
			}
			return list;
		}
	}
	Py_RETURN_NONE;
}

PyObject* PyTile::features(PyTile* self)
{
	Py_RETURN_NONE;	// TODO
}

PyObject* PyTile::id(PyTile* self)
{
	Py_RETURN_NONE;
}

PyObject* PyTile::indexes(PyTile* self)
{
	Py_RETURN_NONE; // TODO
}

PyObject* PyTile::is_active(PyTile* self)
{
	// return PyBool_FromLong(self->store && self->store->tileIndex()tile_.is_active());
	Py_RETURN_NONE; // TODO
}

PyObject* PyTile::is_current(PyTile* self)
{
	Py_RETURN_NONE; // TODO
}

PyObject* PyTile::is_loaded(PyTile* self)
{
	Py_RETURN_NONE; // TODO
}


PyObject* PyTile::parent(PyTile* self)
{
	Py_RETURN_NONE; // TODO
}

PyObject* PyTile::revision(PyTile* self)
{
	uint32_t revision = 0;
	if (self->store)
	{
		TilePtr pTile = self->store->fetchTile(self->tip_);
		if (pTile) revision = pTile.revision();
	}
	return PyLong_FromLong(revision);
}

PyObject* PyTile::row(PyTile* self)
{
	return PyLong_FromLong(self->tile_.row());
}

PyObject* PyTile::shape(PyTile* self)
{
	// TODO
	Py_RETURN_NONE;
}

PyObject* PyTile::size(PyTile* self)
{
	uint32_t size = 0;
	if (self->store)
	{
		TilePtr pTile = self->store->fetchTile(self->tip_);
		if (pTile) size = pTile.totalSize();
	}
	return PyLong_FromLong(size);
}

PyObject* PyTile::tip(PyTile* self)
{
	return PyLong_FromLong(self->tip_);
}

PyObject* PyTile::zoom(PyTile* self)
{
	return PyLong_FromLong(self->tile_.zoom());
}

PyTypeObject PyTile::TYPE =
{
	.tp_name = "geodesk.Tile",
	.tp_basicsize = sizeof(PyTile),
	.tp_dealloc = (destructor)dealloc,
	.tp_repr = (reprfunc)repr,
	.tp_hash = (hashfunc)hash,
	.tp_str = (reprfunc)str,
	.tp_getattro = (getattrofunc)getattro,
	.tp_flags = Py_TPFLAGS_DEFAULT, // | Py_TPFLAGS_DISALLOW_INSTANTIATION,
	.tp_doc = "Tile objects",
	.tp_richcompare = (richcmpfunc)richcompare,
	// .tp_iter = (getiterfunc)iter,
	// .tp_iternext = (iternextfunc)next,
};
