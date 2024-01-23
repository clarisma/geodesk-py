// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#include "PyTile.h"
#include "python/geom/PyBox.h"

#include "PyTile_lookup.cxx"

PyTile* PyTile::create(FeatureStore* store, Tile tile, uint32_t tip)
{
	PyTile* self = (PyTile*)TYPE.tp_alloc(&TYPE, 0);
	if (self)
	{
		store->addref();
		self->store = store;
		self->tile = tile;
		self->tip = tip;
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
	return static_cast<Py_hash_t>(self->tile); 
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
	// TODO
	Py_RETURN_NONE;
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
	Tile tile = self->tile;
	return PyUnicode_FromFormat("%d/%d/%d", tile.zoom(), tile.column(), tile.row());
}

PyObject* PyTile::bounds(PyTile* self)
{
	return PyBox::create(self->tile.bounds());
}

PyObject* PyTile::column(PyTile* self)
{
	return PyLong_FromLong(self->tile.column());
}

PyObject* PyTile::id(PyTile* self)
{
	Py_RETURN_NONE;
}

PyObject* PyTile::revision(PyTile* self)
{
	Py_RETURN_NONE;
}

PyObject* PyTile::row(PyTile* self)
{
	return PyLong_FromLong(self->tile.row());
}

PyObject* PyTile::size(PyTile* self)
{
	// pointer tileIndex(self->store->tileIndex());
	// TODO: Don't fetch tile, return 0 if tile is missing
	pointer pTile = self->store->fetchTile(self->tip);
	// TODO: stale/missing tiles
	// TODO: Blob header will change in 2.0
	return PyLong_FromLong(pTile.getUnsignedInt() & 0x3fff'ffff);
}

PyObject* PyTile::zoom(PyTile* self)
{
	return PyLong_FromLong(self->tile.zoom());
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
