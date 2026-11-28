// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#include "PyFeatures.h"
#include <geodesk/feature/FeatureIterator.h>
#include <geodesk/filter/ComboFilter.h>
#include <geodesk/filter/IntersectsFilter.h>
// #include "match/MatcherDecoder.h"       // TODO: remove (only needed for explain)
#include <geodesk/geom/Area.h>
#include <geodesk/geom/GeometryBuilder.h>
#include <geodesk/geom/Length.h>
#include "python/Environment.h"
#include "python/feature/PyFeature.h"
#include "python/format/PyFormatter.h"
#include "python/format/PyMap.h"
#include "python/geom/PyBox.h"
#include "python/geom/PyCoordinate.h"
#include "python/util/PyFastMethod.h"
#include "PyQuery.h"
#include "PyTile.h"
#include <clarisma/util/Parser.h>

#include "PyFeatures_attr.cxx"
#include "PyFeatures_lookup.cxx"

using namespace clarisma;

PyFeatures* PyFeatures::createNew(PyTypeObject* type, PyObject* args, PyObject* kwds)
{
    Py_ssize_t argCount = PySequence_Length(args);
    if (argCount < 1)
    {
        PyErr_SetString(PyExc_TypeError, "Missing argument <gol_file>");
        return NULL;
    }
    if (argCount == 1 && !kwds)
    {
        PyObject* arg = PyTuple_GetItem(args, 0);
        std::string_view fileName = Python::getStringView(arg);
        if (!fileName.data()) return NULL;
        FeatureStore* store;
        try
        {
            store = FeatureStore::openSingle(fileName);
        }
        catch (const FileNotFoundException& ex)
        {
            PyErr_SetString(PyExc_FileNotFoundError, ex.what());
            return NULL;
        }
        catch (const std::bad_alloc&)
        {
            PyErr_NoMemory();
            return NULL;
        }
        catch (const std::exception& ex)
        {
            PyErr_SetString(PyExc_RuntimeError, ex.what());
            return NULL;
        }
        return create(WORLD, USES_BOUNDS, FeatureTypes::ALL, store,
            Box::ofWorld(), store->getAllMatcher(), nullptr);
    }
    PyErr_SetString(PyExc_TypeError, "Expected single argument (name of GOL file)");
    return NULL;

    // return build(args, kwds); // TODO
}

PyFeatures* PyFeatures::createEmpty()
{
    PyFeatures* self = (PyFeatures*)TYPE.tp_alloc(&TYPE, 0);
    if (self) [[likely]]
    {
        ::new (self) Selection(EMPTY, 0, 0, nullptr,
    {}, nullptr, nullptr);
    }
    return self;
}

void PyFeatures::dealloc(PyFeatures* self)
{
    if(self->store())
    {
        self->release();
    }
    else
    {
        assert(self->matcher() == nullptr);
        assert(self->filter() == nullptr);
    }
    Py_TYPE(self)->tp_free(self);
}

PyObject* PyFeatures::dir(PyFeatures* self, PyObject* args, PyObject* kwargs)
{
    return Python::createList(ATTR_NAMES, ATTR_COUNT);
}

PyObject* PyFeatures::getIter()
{
    switch (kind_)
    {
    case WORLD:
        return PyQuery::create(this);
    case WAY_NODES:
        return PyWayNodeIterator::create(this);
    case MEMBERS:
        return PyMemberIterator::create(this);
    }
    // TODO
}

PyObject* PyFeatures::iter(PyFeatures* self)
{
    return self->getIter();
}

/*
PyObject* PyFeatures::World::getTiles(PyFeatures* self)
{
    PyObject* list = PyList_New(0);
    if (list)
    {
        FeatureStore* store = self->store();
        TileIndexWalker tiw(store->tileIndex(), store->zoomLevels(), self->bounds(), self->filter());
        do
        {
            PyTile* tile = PyTile::create(store, tiw.currentTile(), tiw.currentTip());
            if (tile)
            {
                if (PyList_Append(list, tile) == 0)
                {
                    Py_DECREF(tile);
                    continue;
                }
                Py_DECREF(tile);
            }
            Py_DECREF(list);
            return NULL;
        }
        while (tiw.next());
    }
    return list;
}
*/


PyObject* PyFeatures::call(PyFeatures* self, PyObject* args, PyObject* kwargs)
{
    Py_ssize_t argCount = PyTuple_Size(args);
    if (argCount == 1)
    {
        // single argument
        PyObject* arg = PyTuple_GetItem(args, 0);
        PyTypeObject* type = Py_TYPE(arg);
        if (type == &PyBox::TYPE)
        {
            return self->withBounds(((PyBox*)arg)->box);
        }
        if (type == &PyUnicode_Type)
        {
            const char* query = PyUnicode_AsUTF8(arg);
            if (!query) return nullptr;
            return self->withQuery(query);
        }
        if (type == &TYPE)
        {
            return self->withOther(*(PyFeatures*)arg);
        }
        if (type == &PyFeature::TYPE)
        {
            PyFeature* feature = (PyFeature*)arg;
            IntersectsFilterFactory filterFactory;
            // TODO: check if factory throws or returns null?
            return self->withFilter(filterFactory.forFeature(
                feature->store, feature->feature));
        }
        if (type->tp_name[0] != 'g')    
        {
            // cheap check for potential "shapely" class (i.e. not in "geodesk"
            // module -- unfortunately can't check if name starts with "shapely"
            // because Shapely's geometry classes are regular Python classes,
            // whose name does not include the module prefix
            // TODO: Is there a better way? 

            GEOSGeometry* geom;
            if (Environment::get().getGeosGeometry(arg, &geom))
            {
                GEOSContextHandle_t context = Environment::get().getGeosContext();
                // TODO: Use Filters::intersect(geom)
                IntersectsFilterFactory filterFactory;
                // TODO: check if factory throws or returns null?
                return self->withFilter(filterFactory.forGeometry(context, geom));
            }
        }
        
        // TODO: if (type == &PyCoordinate::TYPE)
        
        if (arg == Py_None) return getEmpty();

        PyErr_Format(PyExc_TypeError, "%s is not a valid argument", type->tp_name);
        return NULL;
    }
    if (argCount == 0)
    {
        // TODO: check kwargs!
        return Python::newRef(self);
    }
    PyErr_SetString(PyExc_TypeError, "Expected query|Box|Features");
    return NULL;
}



int PyFeatures::forEach(FeatureFunction func)
{
    PyObject* iter = getIter();
    if (!iter) return -1;
    for (;;)
    {
        PyFeature* feature = (PyFeature*)PyIter_Next(iter);
        if (feature == NULL) break;
        func(feature);
        Py_DECREF(feature);
    }
    Py_DECREF(iter);
    return PyErr_Occurred() ? -1 : 0;
}


PyObject* PyFeatures::getFirst(bool mustHaveOne, bool mayHaveMore) const
{
    FeatureIterator<Feature> iter(*this);
    if (iter != nullptr)
    {
        PyObject* result;
        if (iter.current().isAnonymousNode())
        {
            result = PyFeature::create(store(), iter.current().id(),
                iter.current().xy());
        }
        else
        {
            result = PyFeature::create(store(), iter.current().ptr(),
                iter.currentRole());
        }
        if (!mayHaveMore)
        {
            ++iter;
            if (iter != nullptr)
            {
                Py_DECREF(result);
                Environment::get().raiseQueryException(
                    "Expected only one feature, but found multiple.");
                // TODO: list them
                return nullptr;
            }
        }
        return result;
    }
    if (mustHaveOne)
    {
        Environment::get().raiseQueryException("No feature found.");
        return nullptr;
    }
    Py_RETURN_NONE;
}

int PyFeatures::isTrue(PyFeatures* self) 
{
    return self->isEmpty() ? 0 : 1;
}

PyObject* PyFeatures::getattr(PyFeatures* self, PyObject* nameObj)
{
    Py_ssize_t len;
    const char* name = PyUnicode_AsUTF8AndSize(nameObj, &len);
    if (!name) return NULL;

    PyFeaturesAttribute* attr = PyFeatures_AttrHash::lookup(name, len);
    if (attr)
    {
        Python::AttrRef ref = attr->attr;
        if (ref.isMethod())
        {
            return PyFastMethod::create(self, ref.method());
        }
        return ref.getter()(self);
    }
    return PyObject_GenericGetAttr(self, nameObj);
    /*
    PyErr_SetString(PyExc_AttributeError, name);
    return NULL;
    */
}

int PyFeatures::contains(PyFeatures* self, PyObject* obj)
{
    if (obj->ob_type == &PyFeature::TYPE)   [[likely]]
    {
        return self->containsFeature((PyFeature*)obj);
    }
    if (obj->ob_type == &PyAnonymousNode::TYPE)
    {
        return self->containsAnonNode((PyAnonymousNode*)obj);
    }
    return 0;
}

PyObject* PyFeatures::subscript(PyFeatures* self, PyObject* key)
{
    if (PySlice_Check(key))
    {
        Py_ssize_t start, stop, step, len;

        if (PySlice_GetIndicesEx(key, PY_SSIZE_T_MAX, &start, &stop, &step, &len) < 0)
        {
            return NULL;
        }
        if (start != 0)
        {
            PyErr_SetString(PyExc_ValueError, "Slice must start at 0");
            return NULL;
        }
        if (step != 1)
        {
            PyErr_SetString(PyExc_ValueError, "Step size must be 1");
            return NULL;
        }
        return self->getList(len);
    }
    if (PyNumber_Check(key))
    {
        long index = PyLong_AsLong(key);
        if (index == -1 && PyErr_Occurred()) return NULL;
        if (index != 0)
        {
            Environment::get().raiseQueryException("Only [0] is allowed.");
            return NULL;
        }
        return self->getFirst(true /* mustHaveOne */, true /* mayHaveMore */);
    }
    PyErr_SetString(PyExc_TypeError, "Must be [0] or [:max_count]");
    return NULL;
}


PyObject* PyFeatures::getList(Py_ssize_t maxLen)
{
    PyObject* list = PyList_New(maxLen);
    if (!list) return NULL;

    // TODO: use negative value to signal error
    Py_ssize_t count = 0;
    PyObject* iter = getIter();
    if (!iter)
    {
        Py_DECREF(list);
        return NULL;
    }
    while (count < maxLen)
    {
        PyObject* feature = PyIter_Next(iter);
        if (!feature) break;
        PyList_SET_ITEM(list, count++, feature); 
        // Note: PyList_SET_ITEM steals a reference
    }

    Py_DECREF(iter);   // we're done with the query
    if (PyErr_Occurred())
    {
        Py_DECREF(list);
        return NULL;
    }
    
    if (count < maxLen)
    {
        if (PyList_SetSlice(list, count, maxLen, NULL) == -1) 
        {
            Py_DECREF(list);
            return NULL;
        }
    }
    return list;
}


PyObject* PyFeatures::op_and(PyFeatures* self, PyObject* other)
{
    if (Py_TYPE(other) != &TYPE)
    {
        Py_RETURN_NOTIMPLEMENTED;
    }
    return self->withOther(*(PyFeatures*)other);
}


PyMappingMethods PyFeatures::MAPPING_METHODS =
{
    .mp_subscript = (binaryfunc)PyFeatures::subscript,
};

PyNumberMethods PyFeatures::NUMBER_METHODS =
{
    .nb_bool = (inquiry)isTrue,
    .nb_and = (binaryfunc)op_and,
};

PySequenceMethods PyFeatures::SEQUENCE_METHODS
{
    .sq_contains = (objobjproc)&contains,
};


PyTypeObject PyFeatures::TYPE =
{
    .tp_name = "geodesk.Features",      
    .tp_basicsize = sizeof(PyFeatures),       
    .tp_dealloc = (destructor)PyFeatures::dealloc, 
    .tp_as_number = &NUMBER_METHODS,
    .tp_as_sequence = &SEQUENCE_METHODS,
    .tp_as_mapping = &MAPPING_METHODS,
    .tp_call = (ternaryfunc)PyFeatures::call, 
    .tp_getattro = (getattrofunc)PyFeatures::getattr, 
    .tp_flags = Py_TPFLAGS_DEFAULT,   
    .tp_doc = "Features objects", 
    .tp_iter = (getiterfunc)PyFeatures::iter,  
    .tp_iternext = (iternextfunc)next,
    // .tp_init = (initproc)PyFeatures::init,   
    .tp_new = (newfunc)createNew
};


// PyFeatures itself is an empty iterator, makes it easy for EMPTY Selection
PyObject* PyFeatures::next(PyFeatures* self)
{
    return NULL;
}


bool PyFeatures::containsFeature(PyFeature* feature) const
{
    if (feature->store != store()) [[unlikely]] return false;
    return Selection::contains(feature->feature);
}

bool PyFeatures::containsAnonNode(PyAnonymousNode* node) const
{
    if (node->store != store()) [[unlikely]] return false;
    return Selection::containsAnonNode(node->xy());
}

// === Properties ===

PyObject* PyFeatures::area(PyFeatures* self)
{
    double totalArea = 0;
    int res = self->forEach([&totalArea](PyObject* item)
    {
        if (Py_TYPE(item) == &PyFeature::TYPE)
        {
            PyFeature* feature = (PyFeature*)item;
            FeaturePtr f = feature->feature;
            if (f.isArea())
            {
                double area;
                if (f.isWay())
                {
                    area = Area::ofWay(WayPtr(f));
                }
                else
                {
                    assert(f.isRelation());
                    area = Area::ofRelation(feature->store, RelationPtr(f));
                }
                totalArea += area;
            }
        }
        // ignore anonymous nodes because their area is zero
    });
    return res == 0 ? PyFloat_FromDouble(totalArea) : NULL;
}

PyObject* PyFeatures::count(PyFeatures* self)
{
    return PyLong_FromUnsignedLongLong(self->Selection::count());
}

PyObject* PyFeatures::first(PyFeatures* self)
{
    return self->getFirst(false /* mustHaveOne */, true /* mayHaveMore */);
}

PyObject* PyFeatures::guid(PyFeatures* self)
{
    char buf[64];
    size_t len = 0;
    if (self->store())  [[likely]]
    {
        self->store()->guid().format(buf);
        len = UUID::FORMATTED_LENGTH;
    }
    return Python::toStringObject(buf, len);
}


PyObject* PyFeatures::length(PyFeatures* self)
{
    // TODO: Can this be confused with "count"?
    // Make it total_length?
    // Would need to change area to total_area

    double totalLength = 0;
    int res = self->forEach([&totalLength](PyObject* item)
    {
        if (Py_TYPE(item) == &PyFeature::TYPE)
        {
            PyFeature* feature = (PyFeature*)item;
            FeaturePtr f = feature->feature;
            if (f.isWay())
            {
                totalLength += Length::ofWay(WayPtr(f));
            }
            else if (f.isRelation())
            {
                totalLength += Length::ofRelation(feature->store, RelationPtr(f));
            }
            // ignore nodes because their length is zero
        }
        // ignore anonymous nodes because their length is zero
    });
    return res==0 ? PyFloat_FromDouble(totalLength) : NULL;
}

PyObject* PyFeatures::list(PyFeatures* self)
{
    // TODO
    Py_RETURN_NONE;
}

PyObject* PyFeatures::map(PyFeatures* self)
{
    return PyMap::create(self);
}

PyObject* PyFeatures::nodes(PyFeatures* self)
{
    return self->withTypes(FeatureTypes::NODES);
}

PyObject* PyFeatures::one(PyFeatures* self)
{
    return self->getFirst(true /* mustHaveOne */, false /* mayHaveMore */);
}

PyObject* PyFeatures::properties(PyFeatures* self)
{
    // TODO
    Py_RETURN_NONE;
}

PyObject* PyFeatures::refcount(PyFeatures* self)
{
    // TODO
    Py_RETURN_NONE;
}

PyObject* PyFeatures::relations(PyFeatures* self)
{
    return self->withTypes(FeatureTypes::RELATIONS);
}

PyObject* PyFeatures::revision(PyFeatures* self)
{
    uint32_t revision = 0;
    if (self->store())  [[likely]]
    {
        revision = self->store()->revision();
    }
    return PyLong_FromUnsignedLong(revision);
}

PyObject* PyFeatures::shape(PyFeatures* self)
{
    Environment& env = Environment::get();
    GEOSContextHandle_t geosContext = env.getGeosContext();
    if (!geosContext) return NULL;

    std::vector< GEOSGeometry*> geoms;
    int res = self->forEach([&geoms, geosContext](PyObject* item)
        {
            if (Py_TYPE(item) == &PyFeature::TYPE)
            {
                PyFeature* feature = (PyFeature*)item;
                geoms.push_back(GeometryBuilder::buildFeatureGeometry(
                    feature->store, feature->feature, geosContext));
            }
            else
            {
                assert(Py_TYPE(item) == &PyAnonymousNode::TYPE);
                PyAnonymousNode* node = (PyAnonymousNode*)item;
                geoms.push_back(GeometryBuilder::buildPointGeometry(
                    node->x_, node->y_, geosContext));
            }
        });
    if (res < 0)
    {
        for (GEOSGeometry* geom : geoms) 
        {
            GEOSGeom_destroy_r(geosContext, geom);
        }
        return NULL;
    }
    GEOSGeometry* collection = GEOSGeom_createCollection_r(
        geosContext, GEOS_GEOMETRYCOLLECTION, geoms.data(), 
        static_cast<unsigned int>(geoms.size()));
    return env.buildShapelyGeometry(collection);
}

PyObject* PyFeatures::strings(PyFeatures* self)
{
    if (!self->store()) [[unlikely]]
    {
        return PyList_New(0);
    }
    const StringTable& strings = self->store()->strings();
    uint32_t count = strings.stringCount();
    PyObject* list = PyList_New(count);
    if (!list) return NULL;
    for (uint32_t i = 0; i < count; ++i) 
    {
        PyObject* str = strings.getStringObject(i);
        if(!str)
        {
            Py_DECREF(list);
            return NULL;
        }
        PyList_SetItem(list, i, str);
    }
    return list;
}

PyObject* PyFeatures::indexed_keys(PyFeatures* self)
{
    if (!self->store()) [[unlikely]]
    {
        return PyList_New(0);
    }
    const StringTable& strings = self->store()->strings();
    const IndexedKeyMap& keysToCategories =
        self->store()->snapshot()->keysToCategories();
    PyObject* list = PyList_New(keysToCategories.size());
    if (!list) return NULL;
    int i = 0;
    for (auto it = keysToCategories.begin(); it != keysToCategories.end(); ++it)
    {
        PyObject* str = strings.getStringObject(it->first);
        if (!str)
        {
            Py_DECREF(list);
            return NULL;
        }
        PyList_SetItem(list, i++, str);
    }
    return list;
}


PyObject* PyFeatures::getTiles(PyFeatures* self)
{
    return PyList_New(0);
}

PyObject* PyFeatures::tiles(PyFeatures* self)
{
    // TODO
    Py_RETURN_NONE;
}


PyObject* PyFeatures::timestamp(PyFeatures* self)
{
    DateTime ts;
    if (self->store()) [[likely]]
    {
        ts = self->store()->revisionTimestamp();
    }
    return Environment::get().datetime(ts);
}

PyObject* PyFeatures::ways(PyFeatures* self)
{
    return self->withTypes(FeatureTypes::WAYS);
}


PyObject* PyFeatures::auto_load(PyFeatures* self, PyObject* args, PyObject* kwargs)
{
    PyErr_SetString(PyExc_NotImplementedError,
        "auto_load will be available in Version 0.2.0");
    return NULL;
}

PyObject* PyFeatures::load(PyFeatures* self, PyObject* args, PyObject* kwargs)
{
    // TileLoader loader(self->store);
    // loader.load();
    // TileCompiler compiler(self->store);
    // compiler.compile();
    Py_RETURN_NONE;
    /*
    PyErr_SetString(PyExc_NotImplementedError,
        "load will be available in Version 0.2.0");
    return NULL;
    */

}

PyObject* PyFeatures::update(PyFeatures* self, PyObject* args, PyObject* kwargs)
{
    PyErr_SetString(PyExc_NotImplementedError,
        "update will be available in Version 0.2.0");
    return NULL;
}


PyObject* PyFeatures::explain(PyFeatures* self, PyObject* args, PyObject* kwargs)
{
    /*
    Py_ssize_t argCount = PyTuple_Size(args);
    if (argCount == 1)
    {
        // single argument
        PyObject* arg = PyTuple_GetItem(args, 0);
        PyTypeObject* type = Py_TYPE(arg);
        if (type == &PyUnicode_Type)
        {
            Py_ssize_t len;
            const char* query = PyUnicode_AsUTF8AndSize(arg, &len);
            if (!query) return NULL;
            const MatcherHolder* m = self->store->getMatcher(query);
            
            // TODO: broken, only works for bytecode matchers, not for
            // templated matchers
            DynamicBuffer buf(1024);
            BufferWriter out(&buf);
            MatcherDecoder decoder(self->store, out,
                reinterpret_cast<const uint16_t*>(
                    reinterpret_cast<const uint8_t*>(&m->mainMatcher()) + sizeof(Matcher)));
            decoder.decode();
            PyObject* str = PyUnicode_FromStringAndSize(buf.data(), buf.length());
            m->release();
            return str;
        }
    }
    */
    PyErr_SetString(PyExc_TypeError, "Expected query");
    return NULL;
}

// Lookup by ID

PyObject* PyFeatures::node(PyFeatures* self, PyObject* args, PyObject* kwargs)
{
    return self->findById(FeatureType::NODE, args, kwargs);
}

PyObject* PyFeatures::way(PyFeatures* self, PyObject* args, PyObject* kwargs)
{
    return self->findById(FeatureType::WAY, args, kwargs);
}

PyObject* PyFeatures::relation(PyFeatures* self, PyObject* args, PyObject* kwargs)
{
    return self->findById(FeatureType::RELATION, args, kwargs);
}

class FeatureIdFilter : public Filter
{
public:
    // We don't need to set acceptedTypes because this Filter
    // will never be combined with others; it is only used for
    // finding ways that contain a specific node
    FeatureIdFilter(TypedFeatureId typedId, const Filter* filter) :
        typedId_(typedId),
        secondaryFilter_(filter)
    {
    }

    const Filter* secondaryFilter() const { return secondaryFilter_; }

    bool accept(const FeatureStore* store, FeaturePtr feature, FastFilterHint fast) const override
    {
        if (feature.typedId() != typedId_) return false;
        return !secondaryFilter_ || secondaryFilter_->accept(store, feature, fast);
    }

private:
    TypedFeatureId typedId_;
    const Filter* secondaryFilter_;
};

PyObject* PyFeatures::findById(FeatureType type, PyObject* args, PyObject* kwargs) const
{
    PyObject* arg = Python::checkSingleArg(args, kwargs, "id");
    if (!arg) return NULL;
    uint64_t id = PyLong_AsUnsignedLongLong(arg);
    if (id == static_cast<unsigned long long>(-1) && PyErr_Occurred())
    {
        return NULL;
    }

    static FeatureTypes TYPES[] =
    {
        FeatureTypes::NODES,
        FeatureTypes::WAYS,
        FeatureTypes::RELATIONS
    };

    FeatureTypes types = TYPES[static_cast<int>(type)] & types_;
    if (types == 0)
    {
        Py_RETURN_NONE;
    }

    TypedFeatureId typedId = TypedFeatureId::ofTypeAndId(type, id);
    if (kind() == WORLD)
    {
        FeatureIdFilter idFilter(typedId, filter());
        Query query(store(), bounds(), types, matcher(), &idFilter);
        FeaturePtr feature = query.next();
        if (feature.isNull())
        {
            Py_RETURN_NONE;
        }
        return PyFeature::create(store(), feature, Py_None);
    }
    // TODO: Other views (iterate over PyQuery)

    Py_RETURN_NONE;
}
