// Copyright (c) 2023 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#include "PyTags.h"
#include <common/util/log.h>
#include "feature/FeatureStore.h"
#include "feature/TagIterator.h"
#include "format/GeoJsonWriter.h"

PyObject* PyTags::create(FeatureStore* store, TagsRef tags)
{
    PyTags* self = (PyTags*)TYPE.tp_alloc(&TYPE, 0);
    if (self != nullptr)
    {
        store->addref();
        self->store = store;
        self->tags = tags;
    }
    return (PyObject*)self;
}

void PyTags::dealloc(PyTags* self)
{
    self->store->release();
    Py_TYPE(self)->tp_free(self);
}

PyObject* PyTags::str(PyTags* self)
{
    DynamicBuffer buf(1024);
    TagIterator iter(self->tags, self->store->strings());
    GeoJsonWriter writer(&buf);
    writer.pretty(false);
    writer.writeTags(iter);
    writer.flush();
    return PyUnicode_FromStringAndSize(buf.data(), buf.length());
}

PyObject* PyTags::iter(PyTags* self)
{
    return PyTagIterator::create(self->store, self->tags);
}


Py_ssize_t PyTags::len(PyTags* self)
{
    return self->tags.count();
}

PyObject* PyTags::subscript(PyTags* self, PyObject* keyObj)
{
    if (!PyUnicode_Check(keyObj))
    {
        PyErr_SetString(PyExc_TypeError, "Key must be a string");
        return NULL;
    }
    return self->tags.getValue(keyObj, self->store->strings());
}

PyMappingMethods PyTags::MAPPING_METHODS =
{
    (lenfunc)len,         // mp_length
    (binaryfunc)subscript, // mp_subscript
    nullptr          // mp_ass_subscript
};

PyTypeObject PyTags::TYPE =
{
    .tp_name = "geodesk.Tags",
    .tp_basicsize = sizeof(PyTags),
    .tp_dealloc = (destructor)dealloc,
    .tp_repr = (reprfunc)str,
    .tp_as_mapping = &MAPPING_METHODS,
    // tp_hash 
    .tp_str = (reprfunc)str,
    .tp_flags = Py_TPFLAGS_DEFAULT, // | Py_TPFLAGS_DISALLOW_INSTANTIATION,
    .tp_iter = (getiterfunc)iter,
};


void PyTagIterator::dealloc(PyTagIterator* self)
{
    self->store->release();
    Py_TYPE(self)->tp_free(self);
}

/**
 * Creates a Python tuple that represents a tag. 
 * - If `key` is NULL, return NULL
 * - Otherwise, reference to `key` is "stolen" (i.e. no need for the 
 *   caller to drecrease refcount)
 */
PyObject* PyTagIterator::createTag(PyTagIterator* self, PyObject* key, uint64_t tagVal)
{
    if (!key) return NULL;
    PyObject* value = self->tags.valueAsObject(tagVal, self->store->strings());
    if (!value)
    {
        Py_DECREF(key);
        return NULL;
    }
    PyObject* tag = PyTuple_Pack(2, key, value);
    Py_DECREF(key);
    Py_DECREF(value);
    return tag;
}


/**
 * Returns the next global-key tag. 
 * Before:
 * - `current` must point to this tag
 * After:
 * - If there are more global-key tags, current will point to the next,
 *   and the rerieval method will remain `nextGlobal`
 * - If this was the last global-key tag:
 *   - `current` will be undefined
 *   - If there are local tags, rerieval method will be `firstLocal` 
 *   - If there are no local tags, rerieval method will be `done`
 *   
 */
PyObject* PyTagIterator::nextGlobal(PyTagIterator* self)
{
    static NextTagFunc const NEXT[] =
    {
        &PyTagIterator::nextGlobal,
        &PyTagIterator::nextGlobal,
        &PyTagIterator::done,
        &PyTagIterator::firstLocal
    };

    uint32_t tag = self->current.getUnalignedUnsignedInt();
    int64_t tagVal = (static_cast<int64_t>(self->tags.pointerOffset(
        self->current) + 2) << 32) | tag;
    self->current += 4 + (tag & 2);
    
    // Based on the last-item flag (Bit 15) and the local-tags flag
    // (Bit 0) of the tag-table pointer, the retrieval method will
    // be `nextGlobal`, `firstLocal` or `done`
    self->func = NEXT[((tag >> 14) & 2) + self->tags.hasLocalKeys()];

    int keyCode = (tag >> 2) & 0x1fff;
    PyObject* keyObj = self->store->strings().getStringObject(keyCode);
    return createTag(self, keyObj, tagVal);
}

PyObject* PyTagIterator::firstLocal(PyTagIterator* self)
{
    self->current = self->tags.ptr();
    self->current -= 6;
    return nextLocal(self);
}

PyObject* PyTagIterator::nextLocal(PyTagIterator* self)
{
    static NextTagFunc const NEXT[] =
    {
        &PyTagIterator::nextLocal,
        &PyTagIterator::done
    };

    pointer origin = self->tags.alignedBasePtr();
    int64_t tag = self->current.getUnalignedLong();
    int32_t rawPointer = static_cast<int32_t>(tag >> 16);
    int32_t flags = rawPointer & 7;
    // local keys are relative to the 4-byte-aligned tagtable address
    LocalString keyString(origin + ((rawPointer ^ flags) >> 1));
    int64_t tagVal = (static_cast<int64_t>(self->tags.pointerOffset(
        self->current) - 2) << 32) | ((tag & 0xffff) << 16) | flags;
    self->current -= 6 + (flags & 2);
    
    // Based on the last-item flag (Bit 2), the retrieval method will
    // be `nextLocal` or `done`
    self->func = NEXT[(flags & 4) >> 2];

    PyObject* keyObj = keyString.toStringObject();
    return createTag(self, keyObj, tagVal);
}

PyObject* PyTagIterator::done(PyTagIterator* self)
{
    return NULL;
}

PyObject* PyTagIterator::next(PyTagIterator* self)
{
    return self->func(self);
}

PyObject* PyTagIterator::create(FeatureStore* store, TagsRef tags)
{
    PyTagIterator* self = (PyTagIterator*)TYPE.tp_alloc(&TYPE, 0);
    if (self != nullptr)
    {
        store->addref();
        self->store = store;
        self->tags = tags;
        pointer p = tags.ptr();
        self->current = p;
        if (p.getUnsignedInt() != TagsRef::EMPTY_TABLE_MARKER)
        {
            self->func = &nextGlobal;
        }
        else
        {
            self->func = tags.hasLocalKeys() ? &firstLocal : &done;
        }
    }
    return (PyObject*)self;
}

PyTypeObject PyTagIterator::TYPE =
{
    .tp_name = "geodesk.TagIterator",
    .tp_basicsize = sizeof(PyTagIterator),
    .tp_dealloc = (destructor)dealloc,
    .tp_flags = Py_TPFLAGS_DEFAULT, // | Py_TPFLAGS_DISALLOW_INSTANTIATION,
    .tp_iternext = (iternextfunc)next,
};



