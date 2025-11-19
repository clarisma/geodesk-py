// Copyright (c) 2025 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#include "PyChangedChildren.h"
#include <geodesk/feature/WayNodeIterator.h>
#include <python/feature/PyFeature.h>
#include <python/query/PyFeatures.h>
#include "PyChangedFeature.h"
#include "Changeset.h"


PyChangedFeature* PyChangedChildren::promoteChild(Changeset* changes, PyObject *obj, bool withRole)
{
	PyChangedFeature* child = changes->tryModify(obj);
    if(child) return child;
    if(PyErr_Occurred()) return nullptr;
	PyObject *seq = PySequence_Fast(obj,
         withRole ?
			"Expected feature, coordinate or member tuple" :
			"Expected feature or coordinate");
	if (!seq) return nullptr;
	Py_ssize_t n = PySequence_Fast_GET_SIZE(seq);
	if (n != 2)
	{
		Py_DECREF(seq);
		PyErr_SetString(PyExc_TypeError,
        	withRole ?
	            "Expected (feature,role) or coordinate" :
                "Expected coordinate");
		return nullptr;
	}
	PyObject* first = PySequence_Fast_GET_ITEM(seq, 0);
	PyObject* second = PySequence_Fast_GET_ITEM(seq, 1);
	Py_DECREF(seq); // TODO: safe ???
	if (withRole && PyUnicode_Check(second))
	{
		// TODO: also allow (role,feature) ?
		child = promoteChild(changes, first, false);
        if(!child) return nullptr;
		return PyChangedFeature::createMember(child, second);
	}
	return changes->createNode(first, second);
}


// steals ref to list
PyChangedChildren* PyChangedChildren::create(Changeset* changes, PyObject* list, bool forRelation)
{
	PyChangedChildren* self = (PyChangedChildren*)TYPE.tp_alloc(&TYPE, 0);
	if (self)	[[likely]]
	{
		changes->addref();
		self->changes_ = clarisma::TaggedPtr<Changeset,1>(changes, forRelation);
		self->list_ = list;
	}
	else
	{
		Py_DECREF(list);
	}
	return self;
}

PyChangedChildren* PyChangedChildren::create(Changeset* changes, bool forRelation)
{
	PyObject* list = PyList_New(0);
	if (!list) return nullptr;
	return create(changes, list, forRelation);
}

PyChangedChildren* PyChangedChildren::create(Changeset* changes, PyFeature* parent)
{
	PyObject* list;
	FeatureStore* store = parent->store;
	bool forRelation;
	if (parent->feature.isWay())	[[likely]]
	{
		forRelation = false;
		WayNodeIterator iter(store, WayPtr(parent->feature),
			true, store->hasWaynodeIds());
		int count = iter.remaining();
		list = PyList_New(count);
		if (!list) return nullptr;
		for (int i=0;i<count;i++)
		{
			PyChangedFeature* node;
			WayNodeIterator::WayNode wayNode = iter.next();
			if (wayNode.feature.isNull())	[[likely]]
			{
				node = changes->modify(store, wayNode.id, wayNode.xy);
			}
			else
			{
				PyFeature* featureNode = PyFeature::create(store, wayNode.feature, Py_None);
				node = changes->modify(featureNode);
				Py_DECREF(featureNode);
			}
			if (!node)	[[unlikely]]
			{
				Py_DECREF(list);
				return nullptr;
			}
			PyList_SET_ITEM(list, i, node);  // steals reference to node
		}
	}
	else
	{
		assert(parent->feature.isRelation());
		forRelation = true;
		// TODO
	}
	return create(changes, list, forRelation);
}

PyChangedChildren* PyChangedChildren::fromSequence(Changeset* changes, PyObject* seq, bool forRelation)
{
	Py_ssize_t n = PySequence_Fast_GET_SIZE(seq);
	PyObject **items = PySequence_Fast_ITEMS(seq);
	PyObject* list = PyList_New(n);

	for (int i=0; i<n; i++)
	{
		PyChangedFeature* changed = promoteChild(changes, items[i], true);
		if (!changed)
		{
			Py_DECREF(list);
			return nullptr;
		}
		if (!forRelation)
		{
			if (changed->type() != PyChangedFeature::NODE)
			{
				forRelation = true;
				// Upgrade any preceding nodes to member
				for (int i2=0; i2<i; i2++)
				{
					PyChangedFeature* prev = (PyChangedFeature*)
						PyList_GET_ITEM(list, i2);
					assert(prev->type() == PyChangedFeature::NODE);
					PyList_SET_ITEM(list, i2,
						PyChangedFeature::createMember(prev, Py_None));
				}
			}
		}
		if (forRelation && changed->type() != PyChangedFeature::MEMBER)
		{
			changed = PyChangedFeature::createMember(changed, Py_None);
		}
		PyList_SET_ITEM(list, i, changed);
	}
	return create(changes, list, forRelation);
}

void PyChangedChildren::dealloc(PyChangedChildren* self)
{
	self->changes()->release();
	Py_DECREF(self->list_);
	Py_TYPE(self)->tp_free(self);
}


PyObject* PyChangedChildren::getitem(PyChangedChildren* self, PyObject* key)
{
	return PyObject_GetItem(self->list_, key);
}


int PyChangedChildren::setitem(PyChangedChildren* self,
    PyObject* key, PyObject* value)
{
    // Slice case: obj[start:stop:step] or del obj[start:stop:step]
    if (PySlice_Check(key))
    {
        // Deletion: del obj[slice]
        if (!value)
        {
            // Capture old items for callbacks
            PyObject* old = PyObject_GetItem(self->list_, key);
            if (!old) return -1;
            if (PyObject_DelItem(self->list_, key) < 0)
            {
                Py_DECREF(old);
                return -1;
            }

            // Call childRemoved() for each old element
            PyObject* it = PyObject_GetIter(old);
            if (!it)
            {
                Py_DECREF(old);
                return -1;
            }

            PyObject* item = nullptr;
            while ((item = PyIter_Next(it)))
            {
                childRemoved(
                    reinterpret_cast<PyChangedFeature*>(item));
                Py_DECREF(item);
            }

            Py_DECREF(it);
            Py_DECREF(old);

            return PyErr_Occurred() ? -1 : 0;
        }

        // Assignment: obj[slice] = value
        // 1. Capture old items for callbacks
        PyObject* old = PyObject_GetItem(self->list_, key);
        if (!old) return -1;

        // 2. Promote all new items first, using accept()
        PyObject* it = PyObject_GetIter(value);
        if (!it)
        {
            Py_DECREF(old);
            return -1;
        }

        std::vector<PythonRef<PyChangedFeature>> accepted;
        for (;;)
        {
            PyObject* elem = PyIter_Next(it); // new ref or nullptr
            if (!elem) break;
            PyChangedFeature* child = self->accept(elem);
            Py_DECREF(elem);

            if (!child)
            {
                Py_DECREF(it);
                Py_DECREF(old);
                // No changes to list yet; just return error
                return -1;
            }
            accepted.emplace_back(PythonRef(child));
        }
        Py_DECREF(it);

        if (PyErr_Occurred())
        {
            Py_DECREF(old);
            return -1;
        }

        // 3. Build a new list from promoted children
        Py_ssize_t n = accepted.size();
        PyObject* new_list = PyList_New(n);
        if (!new_list)
        {
            Py_DECREF(old);
            return -1;
        }

        for (Py_ssize_t i = 0; i < n; ++i)
        {
            PyObject* child = accepted[i].release();
            // PyList_SET_ITEM steals the reference
            PyList_SET_ITEM(new_list, i, child);
        }
        // At this point, promoted vector elements are owned by new_list.

        // 4. Perform the slice assignment
        int rc = PyObject_SetItem(self->list_, key, new_list);
        Py_DECREF(new_list);
        if (rc < 0)
        {
            // Slice assignment failed; old list is unchanged.
            Py_DECREF(old);
            return -1;
        }

        // 5. Notify removed for old slice
        PyObject* old_it = PyObject_GetIter(old);
        if (!old_it)
        {
            Py_DECREF(old);
            return -1;
        }

        PyObject* item = nullptr;
        while ((item = PyIter_Next(old_it)))
        {
            childRemoved(
                reinterpret_cast<PyChangedFeature*>(item));
            Py_DECREF(item);
        }
        Py_DECREF(old_it);
        Py_DECREF(old);

        if (PyErr_Occurred()) return -1;

    	/*

    	 // TODO: we've released the refs at this point,
    	 // so this won't work

        // 6. Notify added for new slice
        for (PyObject* child_obj : promoted)
        {
            PyChangedChildren::childAdded(
                reinterpret_cast<PyChangedFeature*>(child_obj));
            // Do NOT DECREF here; list now owns these refs.
        }
		*/

        return 0;
    }

    // Non-slice: treat as an index (or raise IndexError)
    Py_ssize_t index =
        PyNumber_AsSsize_t(key, PyExc_IndexError);
    if (index == -1 && PyErr_Occurred())
    {
        return -1;
    }

    // Delegate to sequence assignment logic (handles delete and assign)
    return seqAssItem(self, index, value);
}


PyObject* PyChangedChildren::iter(PyChangedChildren* self)
{
	return PyObject_GetIter(self->list_);
}

PyObject* PyChangedChildren::repr(PyChangedChildren* self)
{
	return PyObject_Repr(self->list_);
}

PyObject* PyChangedChildren::richcompare(PyChangedChildren* self, PyObject* other, int op)
{
	return PyObject_RichCompare(self->list_, other, op);
}

PyObject* PyChangedChildren::str(PyChangedChildren* self)
{
	return PyObject_Str(self->list_);
}


PyChangedFeature* PyChangedChildren::accept(PyObject* obj)
{
	bool forRelation = containsRelationMembers();
	PyChangedFeature* changed = promoteChild(changes(), obj, forRelation);
	if (!changed) return nullptr;
	if (forRelation)
	{
		if (!changed->isMember())
		{
			changed = PyChangedFeature::createMember(changed, Py_None);
		}
	}
	else
	{
		if (changed->type() != PyChangedFeature::NODE)
		{
			PyErr_SetString(PyExc_TypeError, "Expected node");
			Py_DECREF(changed);
			return nullptr;
		}
	}
	return changed;
}

PyObject* PyChangedChildren::append(PyChangedChildren* self, PyObject* arg)
{
	PyChangedFeature* child = self->accept(arg);
	if (!child) return nullptr;
	if (PyList_Append(self->list_, child) < 0)
	{
		Py_DECREF(child);
		return nullptr;
	}
	childAdded(child);
	Py_DECREF(child);
	Py_RETURN_NONE;
}

/// @brief Extend with children from an iterable.
/// @details Promotes all items first; if any promotion fails,
/// list is unchanged.
///
PyObject* PyChangedChildren::extend(PyChangedChildren* self, PyObject* arg)
{
    PyObject* it = PyObject_GetIter(arg);
    if (!it) return nullptr;

    std::vector<PythonRef<PyChangedFeature>> accepted;
    for (;;)
    {
        PyObject* item = PyIter_Next(it); // new ref or nullptr
        if (!item) break;

        PyChangedFeature* child = self->accept(item);
        Py_DECREF(item);
        if (!child)
        {
            Py_DECREF(it);
            return nullptr;
        }
        accepted.emplace_back(PythonRef(child));
    }

    Py_DECREF(it);
    if (PyErr_Occurred()) return nullptr;

    // All promotions succeeded; now append to list.
    for (auto& childRef : accepted)
    {
    	PyChangedFeature* child = childRef.get();
        if (PyList_Append(self->list_, child) < 0)
        {
            // At this point the list may be partially updated.
            // Requirement only demands no change on promotion failure,
            // so this is acceptable.
            return nullptr;
        }
        childAdded(child);
    	// childRef.release();		// TODO: check if needed???
    }
    Py_RETURN_NONE;
}

/// @brief Insert a child at a given index.
///
PyObject* PyChangedChildren::insert(PyChangedChildren* self, PyObject* args)
{
    Py_ssize_t index;
    PyObject* obj = nullptr;

    if (!PyArg_ParseTuple(args, "nO", &index, &obj))
    {
        return nullptr;
    }

    PyChangedFeature* child = self->accept(obj);
    if (!child) return nullptr;
    Py_ssize_t len = PyList_GET_SIZE(self->list_);
    if (index < 0) index += len;
    if (index < 0)
    {
        index = 0;
    }
    else if (index > len)
    {
        index = len;
    }

    if (PyList_Insert(self->list_, index, child) < 0)
    {
        Py_DECREF(child);
        return nullptr;
    }
    childAdded(child);
    Py_DECREF(child);
    Py_RETURN_NONE;
}

/// @brief Remove all occurrences of the given object.
/// @details Uses identity comparison (is), not equality.
///
PyObject* PyChangedChildren::remove(PyChangedChildren* self, PyObject* arg)
{
    PyObject* target = arg;
    Py_ssize_t len = PyList_GET_SIZE(self->list_);
    Py_ssize_t write = 0;
    Py_ssize_t removed_count = 0;

    for (Py_ssize_t read = 0; read < len; ++read)
    {
        PyChangedFeature* item = self->borrowAt(read);
        if (item == target)		// TODO: decide on equality
        {
            childRemoved(item);
            ++removed_count;
        }
        else
        {
            if (write != read)
            {
                PyList_SET_ITEM(self->list_, write, item);
            }
            ++write;
        }
    }

	// TODO: review
    if (PyList_SetSlice(self->list_, write, len, nullptr) < 0)
    {
        return nullptr;
    }
    Py_RETURN_NONE;
}

/// @brief Pop and return child at index (default last).
///
PyObject* PyChangedChildren::pop(PyChangedChildren* self, PyObject* args)
{
    Py_ssize_t index = -1;
    if (!PyArg_ParseTuple(args, "|n", &index))
    {
        return nullptr;
    }

    Py_ssize_t len = PyList_GET_SIZE(self->list_);
    if (len == 0)
    {
        PyErr_SetString(PyExc_IndexError, "pop from empty list");
        return nullptr;
    }

    if (index < 0)
    {
        index += len;
    }
    if (index < 0 || index >= len)
    {
        PyErr_SetString(PyExc_IndexError,
            "pop index out of range");
        return nullptr;
    }

    PyChangedFeature* child = self->newAt(index);
	if (PySequence_DelItem(self->list_, index) < 0)
    {
        Py_DECREF(child);
        return nullptr;
    }
    childRemoved(child);
    return child;
}

/// @brief Remove all children.
///
PyObject* PyChangedChildren::clear(PyChangedChildren* self, PyObject* /*ignored*/)
{
    Py_ssize_t len = PyList_GET_SIZE(self->list_);
    for (Py_ssize_t i = 0; i < len; ++i)
    {
        PyChangedFeature* child = self->borrowAt(i);
        childRemoved(child);
    }

    if (PyList_SetSlice(self->list_, 0, len, nullptr) < 0)
    {
        return nullptr;
    }
    Py_RETURN_NONE;
}

/// @brief Reverse the order of children in-place.
PyObject* PyChangedChildren::reverse(PyChangedChildren* self, PyObject* /*ignored*/)
{
    if (PyList_Reverse(self->list_) < 0)
    {
        return nullptr;
    }
    Py_RETURN_NONE;
}

/// @brief Return number of occurrences of value.
/// @details Delegates to the underlying list's count().
///
PyObject* PyChangedChildren::count(
    PyChangedChildren* self,
    PyObject* arg)
{
    PyObject* method =
        PyObject_GetAttrString(self->list_, "count");
    if (!method) return nullptr;

    PyObject* result = PyObject_CallOneArg(method, arg);
    Py_DECREF(method);
    return result;
}

/// @brief Return first index of value; raise ValueError if not found.
/// @details Delegates to the underlying list's index().
///
PyObject* PyChangedChildren::index(
    PyChangedChildren* self, PyObject* args)
{
    PyObject* method =
        PyObject_GetAttrString(self->list_, "index");
	if (!method) return nullptr;

    PyObject* result =
        PyObject_Call(method, args, nullptr);
    Py_DECREF(method);
    return result;
}

Py_ssize_t PyChangedChildren::length(PyObject* self_obj)
{
	auto self = (PyChangedChildren*)self_obj;
	return PyList_GET_SIZE(self->list_);
}

PyObject* PyChangedChildren::seqItem(
	PyObject* self_obj,
	Py_ssize_t index)
{
	auto self = (PyChangedChildren*)self_obj;
	Py_ssize_t len = PyList_GET_SIZE(self->list_);

	if (index < 0)
	{
		index += len;
	}
	if (index < 0 || index >= len)
	{
		PyErr_SetString(PyExc_IndexError, "index out of range");
		return nullptr;
	}
	return self->newAt(index);
}


int PyChangedChildren::seqAssItem(
	PyObject* self_obj,
	Py_ssize_t index,
	PyObject* value)
{
	auto self = (PyChangedChildren*)self_obj;
	Py_ssize_t len = PyList_GET_SIZE(self->list_);
	if (len == 0)
	{
		PyErr_SetString(PyExc_IndexError, "index out of range");
		return -1;
	}

	if (index < 0)
	{
		index += len;
	}
	if (index < 0 || index >= len)
	{
		PyErr_SetString(PyExc_IndexError, "index out of range");
		return -1;
	}

	if (!value)
	{
		// Deletion: del self[index]
		PyChangedFeature* old = self->newAt(index);
		if (PySequence_DelItem(self->list_, index) < 0)
		{
			Py_DECREF(old);
			return -1;
		}
		childRemoved(old);
		Py_DECREF(old);
		return 0;
	}

	// Assignment: self[index] = value
	PyChangedFeature* newChild = self->accept(value);
	if (!newChild) return -1;
	PyChangedFeature* old = self->newAt(index);
	// PyList_SetItem steals reference to new_child on success.
	if (PyList_SetItem(self->list_, index, newChild) < 0)
	{
		Py_DECREF(old);
		Py_DECREF(newChild);
		return -1;
	}

	childRemoved(old);
	childAdded(newChild);

	Py_DECREF(old);
	// no DECREF(new_child); stolen by list
	return 0;
}


int PyChangedChildren::contains(PyObject* self_obj, PyObject* value)
{
	auto self = (PyChangedChildren*)self_obj;
	return PySequence_Contains(self->list_, value);
}


PyMethodDef PyChangedChildren::METHODS[] =
{
	{
		"append",
		(PyCFunction)append,
		METH_O,
		"Append a node/member."
	},
	{
		"extend",
		(PyCFunction)extend,
		METH_O,
		"Extend with nodes/members from an iterable."
	},
	{
		"insert",
		(PyCFunction)insert,
		METH_VARARGS,
		"Insert a child at a given index."
	},
	{
		"remove",
		(PyCFunction)remove,
		METH_O,
		"Remove all occurrences of the given child."
	},
	{
		"pop",
		(PyCFunction)pop,
		METH_VARARGS,
		"Remove and return node/member at index (default last)."
	},
	{
		"clear",
		(PyCFunction)clear,
		METH_NOARGS,
		"Remove all nodes/members."
	},
	{
		"reverse",
		(PyCFunction)reverse,
		METH_NOARGS,
		"Reverse the nodes/members in place."
	},
	{
		"count",
		(PyCFunction)count,
		METH_O,
		"Return the number of occurrences of a node/member."
	},
	{
		"index",
		(PyCFunction)index,
		METH_VARARGS,
		"Return first index of a node/member."
	},
	{ nullptr, nullptr, 0, nullptr }
};



PySequenceMethods PyChangedChildren::SEQUENCE_METHODS =
{
	length,         // sq_length
	nullptr,                           // sq_concat
	nullptr,                           // sq_repeat
	seqItem,        // sq_item
	nullptr,                           // was_sq_slice
	seqAssItem,     // sq_ass_item
	nullptr,                           // was_sq_ass_slice
	contains,       // sq_contains
	nullptr,                           // sq_inplace_concat
	nullptr                            // sq_inplace_repeat
};


PyMappingMethods PyChangedChildren::MAPPING_METHODS =
{
	nullptr,					 // mp_length (optional)
	(binaryfunc)getitem,         // mp_subscript
	(objobjargproc)setitem       // mp_ass_subscript
};

PyTypeObject PyChangedChildren::TYPE =
{
	.tp_name = "geodesk.ChangedChildren",
	.tp_basicsize = sizeof(PyChangedChildren),
	.tp_dealloc = (destructor)dealloc,
	.tp_repr = (reprfunc)repr,
	.tp_as_sequence = &SEQUENCE_METHODS,
	.tp_as_mapping = &MAPPING_METHODS,
	.tp_str = (reprfunc)str,
	.tp_flags = Py_TPFLAGS_DEFAULT, // | Py_TPFLAGS_DISALLOW_INSTANTIATION,
	.tp_doc = "ChangedChildren objects",
	.tp_richcompare = (richcmpfunc)richcompare,
	.tp_iter = (getiterfunc)iter,
	/*
	.tp_methods = METHODS,
	.tp_members = MEMBERS,
	.tp_getset = GETSET,
	*/
};
