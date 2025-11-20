// Copyright (c) 2025 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#include "PyListProxy.h"
#include "python/util/util.h"

// Callback pointers are always assumed to be non-null

PyListProxy* PyListProxy::create(PyObject* list, PyObject* context, const Operations* ops)
{
	assert(PyList_Check(list));
    assert(ops);
	PyListProxy* self = (PyListProxy*)TYPE.tp_alloc(&TYPE, 0);
	if (self)	[[likely]]
	{
		self->list_ = Python::newRef(list);
		self->context_ = Python::newRef(context);
		self->ops_ = ops;
	}
	return self;
}


void PyListProxy::dealloc(PyObject* self_)
{
    auto self = ASSERT_PYTHON_TYPE(self_, PyListProxy);
	Py_DECREF(self->list_);
	Py_DECREF(self->context_);
	Py_TYPE(self)->tp_free(self);
}


PyObject* PyListProxy::listFromIterable(
    PyObject* iterable,
    PyObject* context,
    CoerceFunc coerce)
{
    assert(coerce);
    PyObject* seq = PySequence_Fast(iterable,"expected iterable");
    if (!seq) return nullptr;

    Py_ssize_t n = PySequence_Fast_GET_SIZE(seq);
    PyObject** items = PySequence_Fast_ITEMS(seq);

    PyObject* list = PyList_New(n);
    if (!list)
    {
        Py_DECREF(seq);
        return nullptr;
    }

    for (Py_ssize_t i = 0; i < n; ++i)
    {
        PyObject* src = items[i];
        PyObject* dst = coerce(context, src);
        if (!dst)
        {
            Py_DECREF(seq);
            Py_DECREF(list);
            return nullptr;
        }
        PyList_SET_ITEM(list, i, dst);
    }
    Py_DECREF(seq);
    return list;
}

PyObject* PyListProxy::append(PyObject* list, PyObject* arg,
    PyObject* context, CoerceFunc coerce, ItemFunc added)
{
    assert(coerce);
    assert(added);
    PyObject* item = coerce(context, arg);
    if (!item) return nullptr;
    if (PyList_Append(list, item) < 0)
    {
        Py_DECREF(item);
        return nullptr;
    }
    added(context, list, item);
    Py_DECREF(item);
    Py_RETURN_NONE;
}


/// @brief Extend with children from an iterable.
/// @details Promotes all items first; if any promotion fails,
/// list is unchanged.
///
PyObject* PyListProxy::extend(PyObject* list, PyObject* arg,
    PyObject* context, CoerceFunc coerce, ItemFunc added)
{
    assert(coerce);
    assert(added);
    PyObject* newItems = listFromIterable(arg, context, coerce);
    if (!newItems) return nullptr;

    Py_ssize_t start = PyList_GET_SIZE(list);
    Py_ssize_t n = PyList_GET_SIZE(newItems);

    if (PyList_SetSlice(list, start, start, newItems) < 0)
    {
        Py_DECREF(newItems);
        return nullptr;
    }

    for (Py_ssize_t i = 0; i < n; ++i)
    {
        PyObject* item = PyList_GET_ITEM(newItems, i);
        added(context, list, item);
    }
    Py_DECREF(newItems);
    Py_RETURN_NONE;
}


PyObject* PyListProxy::insert(PyObject* list, PyObject* args,
    PyObject* context, CoerceFunc coerce, ItemFunc added)
{
    assert(coerce);
    assert(added);
    Py_ssize_t index;
    PyObject* value;
    if (!PyArg_ParseTuple(args, "nO:insert", &index, &value))
    {
        return nullptr;
    }

    Py_ssize_t size = PyList_GET_SIZE(list);
    if (index < 0) index += size;
    if (index < 0) index = 0;
    if (index > size) index = size;

    PyObject* item = value;
    item = coerce(context, value);
    if (!item) return nullptr;
    if (PyList_Insert(list, index, item) < 0)
    {
        Py_DECREF(item);
        return nullptr;
    }
    added(context, list, item);
    Py_DECREF(item);
    Py_RETURN_NONE;
}

PyObject* PyListProxy::remove(PyObject* list, PyObject* arg,
    PyObject* context, EqualsFunc equals, ItemFunc removed)
{
    assert(equals);
    assert(removed);
    // Emulate list.remove: find first matching item
    Py_ssize_t size = PyList_GET_SIZE(list);
    for (Py_ssize_t i = 0; i < size; ++i)
    {
        PyObject* item = PyList_GET_ITEM(list, i);
        if (equals(item, arg))
        {
            Py_INCREF(item);
            if (PySequence_DelItem(list, i) < 0)
            {
                Py_DECREF(item);
                return nullptr;
            }
            removed(context, list, item);
            Py_DECREF(item);
            Py_RETURN_NONE;
        }
    }
    PyErr_SetString(PyExc_ValueError, "list.remove(x): x not in list");
    return nullptr;
}

PyObject* PyListProxy::removeAll(PyObject* list, PyObject* arg,
    PyObject* context, EqualsFunc equals, ItemFunc removed)
{
    assert(equals);
    assert(removed);
    Py_ssize_t i = 0;
    Py_ssize_t size = PyList_GET_SIZE(list);
    while (i < size)
    {
        PyObject* item = PyList_GET_ITEM(list, i);
        if (equals(item, arg))
        {
            Py_INCREF(item);
            if (PySequence_DelItem(list, i) < 0)
            {
                Py_DECREF(item);
                return nullptr;
            }
            removed(context, list, item);
            Py_DECREF(item);
            size--;
            continue;
        }
        ++i;
    }
    // No error if nothing was removed
    Py_RETURN_NONE;
}

PyObject* PyListProxy::pop(PyObject* list,  PyObject* args,
    PyObject* context, ItemFunc removed)
{
    assert(removed);
    Py_ssize_t index = -1;
    if (!PyArg_ParseTuple(args, "|n:pop", &index))
    {
        return nullptr;
    }

    Py_ssize_t size = PyList_GET_SIZE(list);
    if (size == 0)
    {
        PyErr_SetString(PyExc_IndexError, "pop from empty list");
        return nullptr;
    }

    if (index < 0) index += size;
    if (index < 0 || index >= size)
    {
        PyErr_SetString(PyExc_IndexError, "pop index out of range");
        return nullptr;
    }

    PyObject* item = Python::newRef(PyList_GET_ITEM(list, index));
    if (PySequence_DelItem(list, index) < 0)
    {
        Py_DECREF(item);
        return nullptr;
    }
    removed(context, list, item);
    return item; // new ref
}

PyObject* PyListProxy::clear(PyObject* list,
    PyObject* context, ItemFunc removed)
{
    assert(removed);
    Py_ssize_t size = PyList_GET_SIZE(list);

    // Collect items first (new refs), because removed() must see the list empty
    std::vector<PyObject*> items;
    items.reserve(size);

    for (Py_ssize_t i = 0; i < size; ++i)
    {
        PyObject* item = PyList_GET_ITEM(list, i);
        Py_INCREF(item);
        items.push_back(item);
    }

    if (PyList_SetSlice(list, 0, PY_SSIZE_T_MAX, nullptr) < 0)
    {
        for (PyObject* item : items)
        {
            Py_DECREF(item);
        }
        return nullptr;
    }

    for (PyObject* item : items)
    {
        removed(context, list, item);
        Py_DECREF(item);
    }
    Py_RETURN_NONE;
}

PyObject* PyListProxy::reverse(PyObject* list,
    PyObject* context, ListFunc reordered)
{
    PyObject* res = PyObject_CallMethod(list, "reverse", nullptr);
    if (!res) return nullptr;
    reordered(context, list);
    return res;
}

PyObject* PyListProxy::count(
    PyObject* list, PyObject* arg, EqualsFunc equals)
{
    assert(equals);
    Py_ssize_t size = PyList_GET_SIZE(list);
    Py_ssize_t c = 0;
    for (Py_ssize_t i = 0; i < size; ++i)
    {
        PyObject* item = PyList_GET_ITEM(list, i);
        if (equals(item, arg)) ++c;
    }
    return PyLong_FromSsize_t(c);
}


int PyListProxy::contains(PyObject* list, PyObject* arg, EqualsFunc equals)
{
    assert(equals);
    Py_ssize_t size = PyList_GET_SIZE(list);
    for (Py_ssize_t i = 0; i < size; ++i)
    {
        PyObject* item = PyList_GET_ITEM(list, i);
        if (equals(item, arg)) return 1;
    }
    return 0;
}


PyObject* PyListProxy::index(PyObject* list, PyObject* args,
    EqualsFunc equals)
{
    assert(equals);
    PyObject* value;
    Py_ssize_t start = 0;
    Py_ssize_t stop = PY_SSIZE_T_MAX;

    if (!PyArg_ParseTuple(args,  "O|nn:index",
        &value, &start, &stop))
    {
        return nullptr;
    }

    Py_ssize_t size = PyList_GET_SIZE(list);
    if (start < 0) start += size;
    if (start < 0) start = 0;
    if (stop < 0) stop += size;
    if (stop > size) stop = size;

    for (Py_ssize_t i = start; i < stop; ++i)
    {
        PyObject* item = PyList_GET_ITEM(list, i);
        if (equals(item, value))
        {
            return PyLong_FromSsize_t(i);
        }
    }

    PyErr_SetString(PyExc_ValueError,
        "list.index(x): x not in list");
    return nullptr;
}


PyObject* PyListProxy::_append(PyObject* self_,  PyObject* arg)
{
    auto self = ASSERT_PYTHON_TYPE(self_, PyListProxy);
    return append(self->list_, arg, self->context_,
        self->ops_->coerceItem, self->ops_->itemAdded);
}

PyObject* PyListProxy::_extend(PyObject* self_,  PyObject* arg)
{
    auto self = ASSERT_PYTHON_TYPE(self_, PyListProxy);
    return extend(self->list_, arg, self->context_,
        self->ops_->coerceItem, self->ops_->itemAdded);
}

PyObject* PyListProxy::_insert(PyObject* self_, PyObject* args)
{
    auto self = ASSERT_PYTHON_TYPE(self_, PyListProxy);
    return insert(self->list_, args, self->context_,
        self->ops_->coerceItem, self->ops_->itemAdded);
}

PyObject* PyListProxy::_remove(PyObject* self_, PyObject* arg)
{
    auto self = ASSERT_PYTHON_TYPE(self_, PyListProxy);
    return remove(self->list_, arg, self->context_,
        self->ops_->itemEquals, self->ops_->itemRemoved);
}

PyObject* PyListProxy::_remove_all(PyObject* self_, PyObject* arg)
{
    auto self = ASSERT_PYTHON_TYPE(self_, PyListProxy);
    return removeAll(self->list_, arg, self->context_,
        self->ops_->itemEquals, self->ops_->itemRemoved);
}

PyObject* PyListProxy::_pop(PyObject* self_, PyObject* args)
{
    auto self = ASSERT_PYTHON_TYPE(self_, PyListProxy);
    return pop(self->list_, args, self->context_,
        self->ops_->itemRemoved);
}

PyObject* PyListProxy::_clear(PyObject* self_, PyObject* /*ignored*/)
{
    auto self = ASSERT_PYTHON_TYPE(self_, PyListProxy);
    return clear(self->list_, self->context_, self->ops_->itemRemoved);
}

PyObject* PyListProxy::_reverse(PyObject* self_, PyObject* /*ignored*/)
{
    auto self = ASSERT_PYTHON_TYPE(self_, PyListProxy);
    return reverse(self->list_, self->context_, self->ops_->listReordered);
}

PyObject* PyListProxy::_count(PyObject* self_, PyObject* arg)
{
    auto self = ASSERT_PYTHON_TYPE(self_, PyListProxy);
    return count(self->list_, arg, self->ops_->itemEquals);
}

int PyListProxy::_contains(PyObject* self_, PyObject* arg)
{
    auto self = ASSERT_PYTHON_TYPE(self_, PyListProxy);
    return contains(self->list_, arg, self->ops_->itemEquals);
}

PyObject* PyListProxy::_index(PyObject* self_, PyObject* args)
{
    auto self = ASSERT_PYTHON_TYPE(self_, PyListProxy);
    return index(self->list_, args, self->ops_->itemEquals);
}


PyObject* PyListProxy::getitem(PyObject* self_, PyObject* key)
{
    auto self = ASSERT_PYTHON_TYPE(self_, PyListProxy);
	return PyObject_GetItem(self->list_, key);
}


int PyListProxy::removeByKey(PyObject* list, PyObject* key,
    PyObject* context, ItemFunc removed)
{
    assert(removed);
    PyObject* old = PyObject_GetItem(list, key);
    if (!old) return -1;

    if (PyObject_DelItem(list, key) < 0)
    {
        Py_DECREF(old);
        return -1;
    }

    // If old is a sequence (e.g. slice), call for each item,
    // otherwise call once.
    if (PySequence_Check(old) && !PyUnicode_Check(old) && !PyBytes_Check(old))
    {
        PyObject* seq = PySequence_Fast(old, nullptr);
        if (seq)
        {
            Py_ssize_t n = PySequence_Fast_GET_SIZE(seq);
            PyObject** items = PySequence_Fast_ITEMS(seq);
            for (Py_ssize_t i = 0; i < n; ++i)
            {
                PyObject* item = items[i];
                removed(context, list, item);
            }
            Py_DECREF(seq);
        }
        else
        {
            PyErr_Clear();
        }
    }
    else
    {
        removed(context, list,old);
    }
    Py_DECREF(old);
    return 0;
}


int PyListProxy::setitem(PyObject* self_, PyObject* key, PyObject* value)
{
    auto self = ASSERT_PYTHON_TYPE(self_, PyListProxy);

    // Deletion (value == nullptr)
    if (!value)
    {
        return removeByKey(self->list_, key,
            self->context_, self->ops_->itemRemoved);
    }

    // Slice assignment: coerce all items via listFromIterable
    if (PySlice_Check(key))
    {
        // Capture old slice
        PyObject* old = PyObject_GetItem(self->list_, key);
        if (!old) return -1;

        PyObject* newList = listFromIterable(value,
            self->context_, self->ops_->coerceItem);
        if (!newList)
        {
            Py_DECREF(old);
            return -1;
        }

        if (PyObject_SetItem(self->list_, key, newList) < 0)
        {
            Py_DECREF(old);
            Py_DECREF(newList);
            return -1;
        }

        // Added items (always call added before removed!)
        PyObject* seq = PySequence_Fast(newList, nullptr);
        if (seq)
        {
            Py_ssize_t n = PySequence_Fast_GET_SIZE(seq);
            PyObject** items = PySequence_Fast_ITEMS(seq);
            for (Py_ssize_t i = 0; i < n; ++i)
            {
                PyObject* item = items[i];
                self->ops_->itemAdded(self->context_,
                    self->list_, item);
            }
            Py_DECREF(seq);
        }
        else
        {
            PyErr_Clear();
        }

        // Removed items
        seq = PySequence_Fast(old, nullptr);
        if (seq)
        {
            Py_ssize_t n = PySequence_Fast_GET_SIZE(seq);
            PyObject** items = PySequence_Fast_ITEMS(seq);
            for (Py_ssize_t i = 0; i < n; ++i)
            {
                PyObject* item = items[i];
                self->ops_->itemRemoved(self->context_,
                    self->list_, item);
            }
            Py_DECREF(seq);
        }
        else
        {
            PyErr_Clear();
        }

        Py_DECREF(old);
        Py_DECREF(newList);
        return 0;
    }

    // Single index assignment
    PyObject* old = PyObject_GetItem(self->list_, key);
    if (!old) return -1;

    PyObject* coerced = self->ops_->coerceItem(self->context_, value);
    if (!coerced)
    {
        Py_DECREF(old);
        return -1;
    }

    if (PyObject_SetItem(self->list_, key, coerced) < 0)
    {
        Py_DECREF(old);
        Py_DECREF(coerced);
        return -1;
    }

    self->ops_->itemAdded(self->context_, self->list_, coerced);
    self->ops_->itemRemoved(self->context_, self->list_, old);
    Py_DECREF(old);
    Py_DECREF(coerced);
    return 0;
}

PyObject* PyListProxy::iter(PyObject* self_)
{
    auto self = ASSERT_PYTHON_TYPE(self_, PyListProxy);
	return PyObject_GetIter(self->list_);
}

PyObject* PyListProxy::repr(PyObject* self_)
{
    auto self = ASSERT_PYTHON_TYPE(self_, PyListProxy);
	return PyObject_Repr(self->list_);
}

PyObject* PyListProxy::richcompare(PyObject* self_, PyObject* other, int op)
{
	auto self = ASSERT_PYTHON_TYPE(self_, PyListProxy);
	return PyObject_RichCompare(self->list_, other, op);
}

PyObject* PyListProxy::str(PyObject* self_)
{
    auto self = ASSERT_PYTHON_TYPE(self_, PyListProxy);
	return PyObject_Str(self->list_);
}

Py_ssize_t PyListProxy::length(PyObject* self_)
{
    auto self = ASSERT_PYTHON_TYPE(self_, PyListProxy);
    return PyList_GET_SIZE(self->list_);
}


PyObject* PyListProxy::seqItem(PyObject* self_, Py_ssize_t index)
{
    auto self = ASSERT_PYTHON_TYPE(self_, PyListProxy);
    return PySequence_GetItem(self->list_, index);
}


int PyListProxy::seqAssItem(PyObject* self_, Py_ssize_t index, PyObject* value)
{
    auto self = ASSERT_PYTHON_TYPE(self_, PyListProxy);

    // Deletion: value == nullptr
    if (!value)
    {
        PyObject* old = PySequence_GetItem(self->list_, index);
        if (!old) return -1;

        if (PySequence_DelItem(self->list_, index) < 0)
        {
            Py_DECREF(old);
            return -1;
        }

        self->ops_->itemRemoved(self->context_, self->list_,old);
        Py_DECREF(old);
        return 0;
    }

    // Assignment: self[index] = value
    PyObject* old = PySequence_GetItem(self->list_, index);
    if (!old) return -1;

    PyObject* coerced = self->ops_->coerceItem(
        self->context_, value);
    if (!coerced)
    {
        Py_DECREF(old);
        return -1;
    }

    if (PySequence_SetItem(self->list_, index, coerced) < 0)
    {
        Py_DECREF(old);
        Py_DECREF(coerced);
        return -1;
    }

    self->ops_->itemAdded(self->context_, self->list_,coerced);
    self->ops_->itemRemoved(self->context_, self->list_,old);
    Py_DECREF(old);
    Py_DECREF(coerced);
    return 0;
}

PyMethodDef PyListProxy::METHODS[] =
{
	{
		"append", _append,	METH_O,
		"Append item"
	},
	{
		"extend", _extend,	METH_O,
		"Extend from iterable"
	},
	{
		"insert", _insert,	METH_VARARGS,
		"Insert at index"
	},
	{
		"remove", _remove,	METH_O,
		"Remove first occurrence"
	},
	{
		"pop", _pop,METH_VARARGS,
		"Remove and return item at index (default last)"
	},
	{
		"clear", _clear, METH_NOARGS,
		"Remove all items"
	},
	{
		"reverse",	_reverse, METH_NOARGS,
		"Reverse in place."
	},
	{
		"count", _count, METH_O,
		"Count occurrences"
	},
	{
		"index", _index,METH_VARARGS,
		"Return first index of an item"
	},
	{ nullptr, nullptr, 0, nullptr }
};



PySequenceMethods PyListProxy::SEQUENCE_METHODS =
{
	length,         // sq_length
	nullptr,                           // sq_concat
	nullptr,                           // sq_repeat
	seqItem,        // sq_item
	nullptr,                           // was_sq_slice
	seqAssItem,     // sq_ass_item
	nullptr,                           // was_sq_ass_slice
	_contains,       // sq_contains
	nullptr,                           // sq_inplace_concat
	nullptr                            // sq_inplace_repeat
};


PyMappingMethods PyListProxy::MAPPING_METHODS =
{
	nullptr,					 // mp_length (optional)
	getitem,         // mp_subscript
	setitem       // mp_ass_subscript
};

PyTypeObject PyListProxy::TYPE =
{
	.tp_name = "geodesk.ListProxy",
	.tp_basicsize = sizeof(PyListProxy),
	.tp_dealloc = dealloc,
	.tp_repr = repr,
	.tp_as_sequence = &SEQUENCE_METHODS,
	.tp_as_mapping = &MAPPING_METHODS,
	.tp_str = str,
	.tp_flags = Py_TPFLAGS_DEFAULT, // | Py_TPFLAGS_DISALLOW_INSTANTIATION,
	.tp_doc = "ListProxy objects",
	.tp_richcompare = richcompare,
	.tp_iter = iter,
	.tp_methods = METHODS,
};
