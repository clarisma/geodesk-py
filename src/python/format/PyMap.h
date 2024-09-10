// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once
#include <Python.h>
#include "format/GeometryWriter.h"
#include "geom/Box.h"
#include "geom/Coordinate.h"
#include <common/alloc/Arena.h>


class PyBinder;
class PyFeature;

class PyMap : public PyObject
{
public:
	enum Attribute
	{
		ATTRIBUTION,
		BASEMAP,
		CLASSNAME,
		COLOR,
		DASHARRAY,
		DASHOFFSET,
		FILL,
		FILLCOLOR,
		FILLOPACITY,
		FILLRULE,
		LEAFLET_STYLESHEET_URL,
		LEAFLET_URL,
		LEAFLET_VERSION,
		LINECAP,
		LINEJOIN,
		LINK,
		MAX_ZOOM,
		MIN_ZOOM,
		OPACITY,
		STROKE,
		TOOLTIP,
		WEIGHT,
		NUMBER_OF_ATTRIBUTES		// marker, must come last
	};

	enum AttributeFlags
	{
		NUM_VALUE = 1,			// value is a number
		BOOL_VALUE = 2,			// value is true or false
		ELEMENT_ATTR = 4,		// attribute applies to each element
		LEAFLET_ATTR = 8		// attribute is passed directly to Leaflet
	};

	PyObject* filename;
	PyObject* attributes[NUMBER_OF_ATTRIBUTES];
	uint64_t globalElementAttributes;
		// a bitset that marks which element attributes are set at the map-level

	static const char* ATTR_NAMES[];
	static const int ATTR_FLAGS[];
	static const char* ATTR_DEFAULTS[];

	static const int DEFAULT_MIN_ZOOM = 0;
	static const int DEFAULT_MAX_ZOOM = 19;
	static const int DEFAULT_PRECISION = 6;
	
	static PyTypeObject TYPE;
	static PyMethodDef METHODS[];
	// static PySequenceMethods SEQUENCE_METHODS;
	// static PyMappingMethods MAPPING_METHODS;

	static PyMap* create(PyObject* obj);
	static PyMap* createEmpty();
	static PyMap* createNew(PyTypeObject* type, PyObject* args, PyObject* kwds);
	static void dealloc(PyMap* self);
	static PyMap* call(PyMap* self, PyObject* args, PyObject* kwargs);
	static PyObject* getattro(PyMap* self, PyObject *attr);
	static int setattro(PyMap* self, PyObject* attr, PyObject* value);
	// static PyObject* iter(PyMap* self);
	static PyObject* str(PyMap* self);

	static PyObject* get_min_zoom(PyMap* self, void* closure);
	static int set_min_zoom(PyMap* self, PyObject* value, void* closure);
	static PyObject* get_max_zoom(PyMap* self, void* closure);
	static int set_max_zoom(PyMap* self, PyObject* value, void* closure);
	static PyObject* get_basemap(PyMap* self, void* closure);
	static int set_basemap(PyMap* self, PyObject* value, void* closure);

	static PyObject* add(PyMap* self, PyObject* args, PyObject* kwargs);
	static PyObject* save(PyMap* self, PyObject* args);
	static PyObject* show(PyMap* self, PyObject* args);

private:
	static const int MAX_ATTR = 20;
	static const char* VALID_ATTR_NAMES[];

	struct ElementAttribute
	{
		int key;
		PyObject* value;
	};

	class Element
	{
	public:
		Element* next;
		uint32_t attrCount;
		PyObject* object;
		ElementAttribute attributes[1];		// variable size

		Element(PyObject* obj, int attrCount);
	};

	int init(PyObject* args, PyObject* kwds);
	int getFilenameFromArgs(PyObject* args);
	int lookupAttr(PyObject* key);
	int setAttribute(PyObject* attr, PyObject* value);
	static PyObject* checkAttributeValue(int key, PyObject* value);
	static PyObject* attributeTypeError(int key);
	const char* stringAttribute(int index);
	const char* getFileName();
	static int setZoom(const char* name, int* zoom, PyObject* value, int defaultValue);
	const char* writeToFile();
	int addObject(PyObject* obj, PyObject* kwargs);
	void releaseItems();

	Arena arena_;
	Element* firstItem_;
	Element** pNextItem_;

	friend class MapWriter;
};


class MapWriter : public GeometryWriter
{
public:
	MapWriter(Buffer* buf, PyMap& map) :
		GeometryWriter(buf),
		map_(map),
		binder_(nullptr)
	{
		latitudeFirst_ = true;
	}

	~MapWriter()
	{
		Py_XDECREF(binder_);
	}
	void writeScript();

private:
	class Schema
	{
	public:
		Schema();
		void fill(const PyMap& map, const PyMap::Element* item);
		void clear();
		void set(int key, PyObject* value);

		PyMap::ElementAttribute attributes[PyMap::MAX_ATTR];
		PyObject* tooltip;
		PyObject* link;
		int attrCount;
		bool hasFormatters;
	};

	void writeItem(PyMap::Element* item);
	void writeObject(PyObject* obj);
	void writePoint(Coordinate c);
	void writeBox(const Box& box);
	void writePolygonOrPolyline(bool polygon);
	void writeWay(WayPtr way);
	void writeRelation(FeatureStore* store, RelationPtr relation);
	void writeRelationMembers(FeatureStore* store, RelationPtr relation, RecursionGuard& guard);
	bool writeFeature(PyFeature* feature);
	void writeGeometry(GEOSContextHandle_t context, const GEOSGeometry* geom);
	void writeGeometryCollection(GEOSContextHandle_t context, const GEOSGeometry* multi);
	void writeAttributeValue(PyObject* value);
	void formatAttributeValue(int key, PyObject* value);
	void initBinder();

	PyMap& map_;
	Box bounds_;
	Schema schema_;
	PyBinder* binder_;
};
