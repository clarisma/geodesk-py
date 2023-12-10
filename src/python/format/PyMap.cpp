// Copyright (c) 2023 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#include "PyMap.h"
#include <structmember.h>	// for PyGetSetDef
#include <string>
#include <filesystem>
#include <cstdlib>
#include <chrono>
#include <ctime>
#include "python/util/PyBinder.h"
#include "python/geom/PyBox.h"
#include "python/geom/PyCoordinate.h"
#include "python/feature/PyFeature.h"
#include "feature/polygon/Polygonizer.h"
#include "feature/FastMemberIterator.h"
#include "geom/Mercator.h"
#include <common/io/File.h>
#include <common/util/BufferWriter.h>
#include <common/util/BitIterator.h>
#include <common/util/log.h>
#include "python/util/util.h"
#include "python/Environment.h"
#include <geos/geom/Geometry.h>

#include "PyMap_attr.cxx"

// TODO: Map should only write to file if modified

const char* PyMap::ATTR_NAMES[] =
{
	"attribution",
	"basemap",
	"classname",
	"color",
	"dashArray",
	"dashOffset",
	"fill",
	"fillColor",
	"fillOpacity",
	"fillrule",
	"leaflet_stylesheet_url",
	"leaflet_url",
	"leaflet_version",
	"lineCap",
	"lineJoin",
	"link",
	"max_zoom",
	"min_zoom",
	"opacity",
	"stroke",
	"tooltip",
	"weight"
};


const int PyMap::ATTR_FLAGS[] =
{
	0, // ATTRIBUTION
	0, // BASEMAP
	ELEMENT_ATTR | LEAFLET_ATTR, // CLASSNAME
	ELEMENT_ATTR | LEAFLET_ATTR, // COLOR
	ELEMENT_ATTR | LEAFLET_ATTR, // DASHARRAY
	ELEMENT_ATTR | LEAFLET_ATTR, // DASHOFFSET
	ELEMENT_ATTR | LEAFLET_ATTR | BOOL_VALUE, // FILL
	ELEMENT_ATTR | LEAFLET_ATTR, // FILLCOLOR
	ELEMENT_ATTR | LEAFLET_ATTR | NUM_VALUE, // FILLOPACITY
	ELEMENT_ATTR | LEAFLET_ATTR, // FILLRULE
	0, // LEAFLET_STYLESHEET_URL
	0, // LEAFLET_URL
	0, // LEAFLET_VERSION
	ELEMENT_ATTR | LEAFLET_ATTR, // LINECAP
	ELEMENT_ATTR | LEAFLET_ATTR, // LINEJOIN
	ELEMENT_ATTR, // LINK
	NUM_VALUE, // MAX_ZOOM
	NUM_VALUE, // MIN_ZOOM
	ELEMENT_ATTR | LEAFLET_ATTR | NUM_VALUE, // OPACITY
	ELEMENT_ATTR | LEAFLET_ATTR | BOOL_VALUE, // STROKE
	ELEMENT_ATTR, // TOOLTIP
	ELEMENT_ATTR | LEAFLET_ATTR | NUM_VALUE, // WEIGHT
};

const char* PyMap::ATTR_DEFAULTS[] =
{
	"Map data &copy; <a href=\"http://openstreetmap.org\">OpenStreetMap</a> contributors", // ATTRIBUTION
	// "https://{s}.tile.openstreetmap.de/tiles/osmde/{z}/{x}/{y}.png", // BASEMAP
	"https://tile.openstreetmap.org/{z}/{x}/{y}.png",  // BASEMAP
	"", // CLASSNAME
	"#3388ff", // COLOR
	"", // DASHARRAY
	"", // DASHOFFSET
	"True", // FILL
	"", // FILLCOLOR
	"0.2", // FILLOPACITY
	"evenodd", // FILLRULE
	"https://unpkg.com/leaflet@{leaflet_version}/dist/leaflet.css", // LEAFLET_STYLESHEET_URL
	"https://unpkg.com/leaflet@{leaflet_version}/dist/leaflet.js", // LEAFLET_URL
	"1.8.0", // LEAFLET_VERSION
	"round", // LINECAP
	"round", // LINEJOIN
	"", // LINK
	"19", // MAX_ZOOM
	"0", // MIN_ZOOM
	"1.0", // OPACITY
	"True", // STROKE
	"", // TOOLTIP
	"3", // WEIGHT
};


const char* PyMap::stringAttribute(int index)
{
	PyObject* value = attributes[index];
	if (!value) return ATTR_DEFAULTS[index];

	// TODO: this assumes string attributes are always stored as string
	return PyUnicode_AsUTF8(value);
}


PyMap* PyMap::createEmpty()
{
	PyMap* self = (PyMap*)TYPE.tp_alloc(&TYPE, 0);
	if (self)
	{
		new (&self->arena_)Arena();
		// Arena doesn't allocate memory in constructor, hence does not throw

		self->filename = NULL;
		memset(self->attributes, 0, sizeof(self->attributes));
		self->globalElementAttributes = 0;
		self->firstItem_ = nullptr;
		self->pNextItem_ = &self->firstItem_;
	}
	return self;
}

PyMap* PyMap::create(PyObject* obj)
{
	PyMap* self = createEmpty();
	if (!self) return NULL;
	if (self->addObject(obj, NULL) < 0)
	{
		Py_DECREF(self);
		return NULL;
	}
	return self;
}

PyMap* PyMap::createNew(PyTypeObject* type, PyObject* args, PyObject* kwargs)
{
	PyMap* self = createEmpty();
	if (self)
	{
		if (self->init(args, kwargs) < 0)
		{
			Py_DECREF(self);
			return NULL;
		}
	}
	return self;
}


int PyMap::getFilenameFromArgs(PyObject* args)
{
	Py_ssize_t argCount = PySequence_Length(args);
	if (argCount > 0)
	{
		if (argCount > 1)
		{
			PyErr_SetString(PyExc_TypeError, "Expected <filename>");
			return -1;
		}
		PyObject* arg = PyTuple_GET_ITEM(args, 0);
		const char* strFilename = PyUnicode_AsUTF8(arg);
		if (strFilename == NULL) return -1;
		const char* ext = File::extension(strFilename);
		if (*ext == 0)
		{
			arg = PyUnicode_FromFormat("%s.html", strFilename);
		}
		else
		{
			Py_INCREF(arg);
		}
		Py_XDECREF(filename);
		filename = arg;
	}
	return 0;
}


int PyMap::init(PyObject* args, PyObject* kwargs)
{
	Py_ssize_t argCount = PySequence_Length(args);
	if (getFilenameFromArgs(args) < 0) return -1;
	if (kwargs)
	{
		PyObject* key;
		PyObject* value;
		Py_ssize_t pos = 0;

		while (PyDict_Next(kwargs, &pos, &key, &value))
		{
			if (setAttribute(key, value) < 0) return -1;
		}
	}
	return 0;
}


void PyMap::dealloc(PyMap* self)
{
	self->releaseItems();
	self->arena_.~Arena();
	Py_XDECREF(self->filename); 
	PyObject** p = self->attributes;
	PyObject** end = p + NUMBER_OF_ATTRIBUTES;
	while (p < end)
	{
		Py_XDECREF(*p++);
	}
}

void PyMap::releaseItems()
{
	Element* p = firstItem_;
	while (p)
	{
		Py_DECREF(p->object);
		for (unsigned int i = 0; i < p->attrCount; i++)
		{
			Py_XDECREF(p->attributes[i].value);
			// value may be NULL if addObject failed
			// TODO: maybe init with dummy object in case of failure?
		}
		p = p->next;
	}
}


PyMap* PyMap::call(PyMap* self, PyObject* args, PyObject* kwargs)
{
	if (self->init(args, kwargs) < 0) return NULL;
	return Python::newRef(self);
}

int PyMap::lookupAttr(PyObject* nameObj)
{
	Py_ssize_t len;
	const char* name = PyUnicode_AsUTF8AndSize(nameObj, &len);
	if (!name) return -1;	
		// TODO: distinguish between error and unknown attr

	Attr* attr = PyMap_AttrHash::lookup(name, len);
	if (!attr) return -1;
	return attr->index;
}

PyObject* PyMap::getattro(PyMap* self, PyObject *attr)
{
	int index = self->lookupAttr(attr);
	if (index < 0) return PyObject_GenericGetAttr(self, attr);
	PyObject* value = self->attributes[index];
	if (value) return Python::newRef(value);
	const char* defaultValue = ATTR_DEFAULTS[index];
	int flags = ATTR_FLAGS[index];
	if (flags & NUM_VALUE)
	{
		return PyFloat_FromDouble(strtod(defaultValue, nullptr));
	}
	if (flags & BOOL_VALUE)
	{
		return Python::boolValue(strcmp(defaultValue, "True") == 0);
	}
	return PyUnicode_FromString(defaultValue);
}

/**
 * Sets a map attribute or a default element attribute.
 * 
 * @param key    the attribute name (must be a string)
 * @param value  must be a string, number or bool, depending
 *               on the attribute type. For element attributes,
 *				 a template string can be passed, even if type
 *               is number or bool (conversion happens later)
 * 
 * @returns 0 on success. 
 * @returns -1 if the attribute does not exists or the value 
 *   is of wrong type (sets exception)
 * .
 */
int PyMap::setAttribute(PyObject* key, PyObject* value)
{
	int index = lookupAttr(key);
	if (index < 0)
	{
		PyErr_SetObject(PyExc_AttributeError, key);
		return -1;
	}

	PyObject* oldValue = attributes[index];
	if (value)
	{
		if (value == Py_None)
		{
			value = NULL;
		}
		else
		{
			value = checkAttributeValue(index, value);
			if (value == NULL) return -1;
		}
	}
	attributes[index] = value;
	Py_XDECREF(oldValue);

	// If the attribute is an element attribute, we mark it as set or cleared
	// in the globalElementAttributes bitfield. 
	int attrBit = (ATTR_FLAGS[index] & ELEMENT_ATTR) ? (static_cast<uint64_t>(1) << index) : 0;
	globalElementAttributes = (value != NULL) ? (globalElementAttributes | attrBit) :
		(globalElementAttributes & (~attrBit));
	return 0;
}


PyObject* PyMap::attributeTypeError(int key)
{
	int flags = ATTR_FLAGS[key];
	const char* expected;
	if (flags & AttributeFlags::NUM_VALUE)
	{
		expected = "a number (or template string)";
	}
	else if (flags & AttributeFlags::BOOL_VALUE)
	{
		expected = "True/False (or template string)";
	}
	else
	{
		expected = "string";
	}
	PyErr_Format(PyExc_TypeError, "%s: Value must be %s", ATTR_NAMES[key], expected);
	return NULL;
}

PyObject* PyMap::checkAttributeValue(int key, PyObject* value)
{
	int flags = ATTR_FLAGS[key];
	if (PyUnicode_Check(value))
	{
		const char* s = PyUnicode_AsUTF8(value);
		if (s == NULL) return NULL;
		if ((flags & ELEMENT_ATTR) && strchr(s, '{'))
		{
			// Value is a template string 
			// Template strings can only be used for element attributes
			// (Map attributes use {} in BASEMAP and LEAFLET_URL, but 
			// these substitutions are handled differently)
			// A template string is valid even if the attribute type is 
			// numeric or boolean (conversion happens after the template
			// is resolved using the individual map elements)
			return PyObject_GetAttrString(value, "format_map");
		}
		if (flags & (AttributeFlags::NUM_VALUE | AttributeFlags::BOOL_VALUE))
		{
			return attributeTypeError(key);
		}
	}
	else if (PyBool_Check(value))
	{
		if ((flags & AttributeFlags::BOOL_VALUE) == 0)
		{
			return attributeTypeError(key);
		}
	}
	else if (PyLong_Check(value) || PyFloat_Check(value))
	{
		if ((flags & AttributeFlags::NUM_VALUE) == 0)
		{
			return attributeTypeError(key);
		}
	}
	else
	{
		return attributeTypeError(key);
	}
	return Python::newRef(value);
}

int PyMap::setattro(PyMap* self, PyObject* attr, PyObject* value)
{
	return self->setAttribute(attr, value);

	// TODO: if attribute is not found, should delegate to
	// PyObject_GenericSetAttr for consistency

	/*
	if (self->setAttribute(attr, value) < 0)
	{
		return PyObject_GenericSetAttr(self, attr, value);
	}
	return 0;
	*/
}


int PyMap::setZoom(const char* name, int* zoom, PyObject* value, int defaultValue)
{
	if (value == NULL)		// "del" resets value to default
	{
		*zoom = defaultValue;
		return 0;
	}
	// Ensure value is a Python int
	if (!PyLong_Check(value)) 
	{
		PyErr_Format(PyExc_TypeError, "%s must be an integer", name);
		return -1;
	}

	long v = PyLong_AsLong(value);
	if (v < 0 || v > 24) 
	{
		PyErr_Format(PyExc_ValueError, "%s must be in range 0 to 24", name);
		return -1;
	}
	*zoom = (int)v;
	return 0;
}


/*
PObject* PyMap::iter(PyMap* self)
{
	// TODO
	Py_RETURN_NONE;
}
*/


PyObject* PyMap::str(PyMap* self)
{
	// TODO
	Py_RETURN_NONE;
}

PyObject* PyMap::add(PyMap* self, PyObject* args, PyObject* kwargs)
{
	Py_ssize_t argCount = PyTuple_Size(args);
	if (argCount < 1)
	{
		PyErr_SetString(PyExc_TypeError, "Missing argument (item to add)");
		return NULL;
	}
	else if (argCount == 1)
	{
		args = PyTuple_GET_ITEM(args, 0);
	}
	// If more than one item is passed, we simple hand off the <args> tuple
	// to addObject (which can deal with sequence objects)

	if (self->addObject(args, kwargs) != 0) return NULL;
	return Python::newRef(self);
}

PyMap::Element::Element(PyObject* obj, int attrCount)
{
	next = nullptr;
	object = Python::newRef(obj);
	this->attrCount = attrCount;
	memset(&attributes, 0, sizeof(ElementAttribute) * attrCount);
}

int PyMap::addObject(PyObject* obj, PyObject* kwargs)
{
	size_t attrCount;
	if (kwargs == 0)
	{
		attrCount = 0;
	}
	else
	{
		attrCount = PyDict_Size(kwargs);
	}

	Element* item = (Element*)arena_.alloc(sizeof(Element) + 
		(attrCount * 2 - 1) * sizeof(ElementAttribute), alignof(Element));
	new (item)Element(obj, attrCount);
	*pNextItem_ = item;
	pNextItem_ = &item->next;

	// TODO: must ensure only Python strings are passed as values!
	//  Or better: convert the values here

	if (attrCount)
	{
		ElementAttribute* p = item->attributes;
		PyObject* key;
		PyObject* value;
		Py_ssize_t pos = 0;

		while (PyDict_Next(kwargs, &pos, &key, &value))
		{
			int index = lookupAttr(key);
			if (index < 0)
			{
				PyErr_SetObject(PyExc_AttributeError, key);
				return -1;
			}
			if ((ATTR_FLAGS[index] & ELEMENT_ATTR) == 0)
			{
				PyErr_Format(PyExc_AttributeError, 
					"%s does not apply to elements", ATTR_NAMES[index]);
				return -1;
			}
			value = checkAttributeValue(index, value);
			if (value == NULL) return -1;
			p->key = index;
			p->value = value;
			p++;
		}
	}
	return 0;
}

PyObject* PyMap::save(PyMap* self, PyObject* args)
{
	if (self->getFilenameFromArgs(args) < 0) return 0;
	if (self->filename == NULL)
	{
		PyErr_SetString(PyExc_RuntimeError, "No file name given");
		return NULL;
	}
	if (self->writeToFile() == NULL) return NULL;
	// return Python::newRef(self);	// TODO: return none
	Py_RETURN_NONE;
}

const char* PyMap::getFileName()
{
	if (filename == NULL)
	{
		std::filesystem::path tempDir = std::filesystem::temp_directory_path();

		using namespace std::chrono;
		// Get the current time
		system_clock::time_point now = system_clock::now();
		time_t tt = system_clock::to_time_t(now);

		// Extract milliseconds
		milliseconds ms = duration_cast<milliseconds>(now.time_since_epoch()) % 1000;

		// Convert to broken-down time (i.e., tm structure)
		tm local_tm = *localtime(&tt);

		// Convert the tm structure to a string without milliseconds
		char buf[100];
		strftime(buf, sizeof(buf), "map_%Y-%m-%d_%H-%M-%S", &local_tm);
		sprintf(&buf[23], "_%03lld.html", ms);
		filename = PyUnicode_FromString((tempDir / buf).string().c_str());
		if (filename == NULL) return NULL;
	}
	return PyUnicode_AsUTF8(filename);
}


PyObject* PyMap::show(PyMap* self, PyObject* args)
{
	const char* fileName = self->writeToFile();
	if (!fileName) return NULL;

#if defined(_WIN32) || defined(_WIN64)
	std::string command = "start " + std::string(fileName);
	system(command.c_str());
#elif defined(__APPLE__)
	std::string command = "open " + std::string(fileName);
	system(command.c_str());
#elif defined(__linux__)
	std::string command = "xdg-open " + std::string(fileName);
	system(command.c_str());
#else
	// Not implemented
#endif
	// return Python::newRef(self);
	Py_RETURN_NONE;
}

const char* PyMap::writeToFile()
{
	const char* fileName = getFileName();
	if (!fileName) return NULL;
	FILE* file = fopen(fileName, "wb");
	if (!file)
	{
		PyErr_Format(PyExc_IOError, "Failed to open %s for writing", fileName);
		return NULL;
	}
	FileBuffer buf(file, 64 * 1024);
	MapWriter out(&buf, *this);

	out.writeConstString(
		"<html><head><meta charset=\"utf-8\">"
		"<link rel=\"stylesheet\" href=\"");
	const char* leafletVersionStr = stringAttribute(LEAFLET_VERSION);
	const char* s = stringAttribute(LEAFLET_STYLESHEET_URL);
	out.writeReplacedString(s, "{leaflet_version}", leafletVersionStr);
	out.writeConstString("\">\n<script src=\"");
	s = stringAttribute(LEAFLET_URL);
	out.writeReplacedString(s, "{leaflet_version}", leafletVersionStr);
	out.writeConstString(
		"\"></script>\n<style>\n#map {height: 100%;}\nbody {margin:0;}\n</style>\n" 
		"</head>\n<body>\n<div id=\"map\"> </div>\n"
		"<script>");
	out.writeScript();
	out.writeConstString("</script></body></html>");
	out.flush();
	// no need to close file, ~FileBuffer does this
	return fileName;
}

PyMethodDef PyMap::METHODS[] =
{
	{"add", (PyCFunction)add, METH_VARARGS | METH_KEYWORDS, "Adds one or more features or shapes to the map" },
	{"save", (PyCFunction)save, METH_VARARGS, "Saves the map as an HTML file" },
	{"show", (PyCFunction)show, METH_VARARGS, "Opens a browser window to display the map" },
	{ NULL, NULL, 0, NULL },
};


/*

PySequenceMethods PyMap::SEQUENCE_METHODS =
{
};

PyMappingMethods PyMap::MAPPING_METHODS =
{
};
*/

PyTypeObject PyMap::TYPE =
{
	.tp_name = "geodesk.Map",
	.tp_basicsize = sizeof(PyMap),
	.tp_dealloc = (destructor)dealloc,
	// .tp_repr = (reprfunc)repr,
	// .tp_as_sequence = &SEQUENCE_METHODS,
	// .tp_as_mapping = &MAPPING_METHODS,
	.tp_call = (ternaryfunc)call,
	.tp_str = (reprfunc)str,
	.tp_getattro = (getattrofunc)getattro,
	.tp_setattro = (setattrofunc)setattro,
	.tp_flags = Py_TPFLAGS_DEFAULT,
	.tp_doc = "Map objects",
	// .tp_iter = (getiterfunc)iter,
	.tp_methods = METHODS,
	.tp_new = (newfunc)createNew,
};


void MapWriter::writeScript()
{
	writeConstString(
		"var map = L.map('map');\n"
		"var tilesUrl='");
	writeString(map_.stringAttribute(PyMap::BASEMAP));
	writeConstString("';\nvar tilesAttrib='");
	writeString(map_.stringAttribute(PyMap::ATTRIBUTION));
	writeConstString(
		"';\nvar tileLayer = new L.TileLayer("
		"tilesUrl, {minZoom: ");
	formatInt(0);		// TODO: MIN_ZOOM
	writeConstString(", maxZoom: ");
	formatInt(19);		// TODO: MAX_ZOOM
	writeConstString(
		", attribution: tilesAttrib});\n"
		"map.setView([51.505, -0.09], 13);\n"      // TODO
		"map.addLayer(tileLayer);\n"
		"L.control.scale().addTo(map);\n");

	PyMap::Element* p = map_.firstItem_;
	while (p)
	{
		writeItem(p);
		p = p->next;
	}

	writeConstString("map.fitBounds([");
	writeCoordinate(Coordinate(bounds_.minX(), bounds_.minY()));
	writeConstString(",");
	writeCoordinate(Coordinate(bounds_.maxX(), bounds_.maxY()));
	writeConstString("]);");
}


void MapWriter::writePoint(Coordinate c)
{
	writeConstString("L.circle(");
	writeCoordinate(c);
}

void MapWriter::writeBox(const Box& box)
{
	writeConstString("L.rectangle([[");
	writeCoordinate(box.topLeft());
	writeConstString("],[");
	writeCoordinate(box.bottomRight());
	writeConstString("]]");
	bounds_.expandToIncludeSimple(box);
}

void MapWriter::writePolygonOrPolyline(bool polygon)
{
	if (polygon)
	{
		writeConstString("L.polygon(");
	}
	else
	{
		writeConstString("L.polyline(");
	}
}

void MapWriter::writeWay(WayRef way)
{
	writePolygonOrPolyline(way.isArea());
	writeWayCoordinates(way);
	// TODO: Leaflet doesn't need duplicate end coordinate for polygons
}

// TODO: don't allow placeholders/empty rels
void MapWriter::writeRelation(FeatureStore* store, RelationRef relation)
{
	if (relation.isArea())
	{
		Polygonizer polygonizer;
		polygonizer.createRings(store, relation);
		polygonizer.assignAndMergeHoles();
		if (!polygonizer.outerRings())
		{
			writePoint(relation.bounds().center());
		}
		else
		{
			writePolygonOrPolyline(true);
			writePolygonizedCoordinates(polygonizer);
		}
	}
	else
	{
		writeConstString("L.featureGroup([");
		RecursionGuard guard(relation);
		writeRelationMembers(store, relation, guard);
		writeByte(']');
	}
	
}

void MapWriter::writeRelationMembers(FeatureStore* store, RelationRef relation, RecursionGuard& guard)
{
	bool first = true;
	FastMemberIterator iter(store, relation);
	for (;;)
	{
		FeatureRef member = iter.next();
		if (member.isNull()) break;
		int memberType = member.typeCode();
		if (memberType == 1)
		{
			WayRef memberWay(member);
			if (memberWay.isPlaceholder()) continue;
			if (!first) writeByte(',');
			writeWay(memberWay);
		}
		else if (memberType == 0)
		{
			NodeRef memberNode(member);
			if (memberNode.isPlaceholder()) continue;
			if (!first) writeByte(',');
			writePoint(memberNode.xy());
		}
		else
		{
			RelationRef childRel(member);
			if (childRel.isPlaceholder() || !guard.checkAndAdd(childRel)) continue;
			if (!first) writeByte(',');
			writeRelation(store, childRel);
		}
		writeByte(')');
		first = false;
	}
}


bool MapWriter::writeFeature(PyFeature* obj)
{
	FeatureRef feature = obj->feature;
	if (feature.isWay())
	{
		WayRef way(feature);
		if (way.isPlaceholder()) return false;
		writeWay(way);
		bounds_.expandToIncludeSimple(way.bounds());
	}
	else if (feature.isNode())
	{
		NodeRef node(feature);
		if (node.isPlaceholder()) return false;
		writePoint(node.xy());
		bounds_.expandToInclude(node.xy());
	}
	else
	{
		assert(feature.isRelation());
		RelationRef relation(feature);
		if (relation.isPlaceholder()) return false;
		writeRelation(obj->store, relation);
		bounds_.expandToIncludeSimple(relation.bounds());
	}
	return true;
}

void MapWriter::writeItem(PyMap::Element* item)
{
	schema_.fill(map_, item);
	if (schema_.hasFormatters) initBinder();
	PyObject* obj = item->object;
	writeObject(obj);
	schema_.clear();
}

void MapWriter::writeGeometry(GEOSContextHandle_t context, const GEOSGeometry* geom)
{
	int geomType = GEOSGeomTypeId_r(context, geom);
	switch (geomType)
	{
	case GEOS_POINT:
		writeConstString("L.circle(");
		writePointCoordinates(context, geom);
		return;
	case GEOS_LINESTRING:
	case GEOS_LINEARRING:
		writeConstString("L.polyline(");
		writeLineStringCoordinates(context, geom);
		return;
	case GEOS_POLYGON:
		writeConstString("L.polygon(");
		writePolygonCoordinates(context, geom);
		return;
	case GEOS_MULTIPOLYGON:
		writeConstString("L.polygon(");
		writeMultiPolygonCoordinates(context, geom);
		return;
	}

	// Geometry collection (other than MultiPolygon)
	writeGeometryCollection(context, geom);
}


void MapWriter::writeGeometryCollection(GEOSContextHandle_t context, const GEOSGeometry* multi)
{
	writeConstString("L.featureGroup([");
	int count = GEOSGetNumGeometries_r(context, multi);
	for (int i = 0; i < count; i++)
	{
		if (i > 0) writeByte(',');
		writeGeometry(context, GEOSGetGeometryN_r(context, multi, i));
		writeByte(')');
	}
	writeByte(']');
}

void MapWriter::writeObject(PyObject* obj)
{
	PyTypeObject* type = Py_TYPE(obj);
	geos::geom::Geometry* geom;

	if (type == &PyFeature::TYPE)
	{
		if (!writeFeature((PyFeature*)obj)) return;
	}
	else if (type == &PyCoordinate::TYPE)
	{
		PyCoordinate* coordObj = (PyCoordinate*)obj;
		Coordinate c(coordObj->x, coordObj->y);
		writePoint(c);
		bounds_.expandToInclude(c);
	}
	else if (type == &PyBox::TYPE)
	{
		PyBox* boxObj = (PyBox*)obj;
		writeBox(boxObj->box);
		bounds_.expandToIncludeSimple(boxObj->box);
	}
	else if (type == &PyAnonymousNode::TYPE)
	{
		PyAnonymousNode* anonNode = (PyAnonymousNode*)obj;
		Coordinate c(anonNode->x_, anonNode->y_);
		writePoint(c);
		bounds_.expandToInclude(c);
	}
	else if (Python::isIterable(obj))
	{
		PyObject* iter = PyObject_GetIter(obj);
		PyObject* childObj;
		while ((childObj = PyIter_Next(iter)))
		{
			writeObject(childObj);
			Py_DECREF(childObj);
		}
		return;		// don't continue to attributes/tooltip/link
	}
	else if (Environment::get().getGeosGeometry(obj, (GEOSGeometry**)&geom))
	{
		GEOSContextHandle_t context = Environment::get().getGeosContext();
		writeGeometry(context, (GEOSGeometry*)geom);
		bounds_.expandToIncludeSimple(Box(geom->getEnvelopeInternal()));
	}
	else
	{
		// TODO: invalid arg, throw
		return;
	}

	if (schema_.attrCount > 0)
	{
		writeConstString(",{");
		for (int i = 0; i < schema_.attrCount; i++)
		{
			if (i > 0) writeByte(',');
			const PyMap::ElementAttribute* attr = &schema_.attributes[i];
			writeString(PyMap::ATTR_NAMES[attr->key]);
			writeByte(':');
			formatAttributeValue(attr->key, attr->value);
		}
		writeByte('}');
	}
	writeByte(')');

	if (schema_.hasFormatters) binder_->addTarget(obj);
	if (schema_.tooltip)
	{
		writeConstString(".bindTooltip(");
		formatAttributeValue(PyMap::TOOLTIP, schema_.tooltip);
		writeConstString(", {sticky: true})");
	}
	if (schema_.link)
	{
		writeConstString(".on('click', function(){window.location=");
		formatAttributeValue(PyMap::LINK, schema_.link);
		writeConstString(";})");
	}
	writeConstString(".addTo(map);\n");
	if (schema_.hasFormatters) binder_->popTarget();
}

void MapWriter::writeAttributeValue(PyObject* value)
{
	if (PyUnicode_Check(value))
	{
		writeByte('\"');
		writeJsonEscapedString(Python::stringAsStringView(value));
			// TODO: this throws!
		writeByte('\"');
	}
	else if (value == Py_True)
	{
		writeConstString("true");
	}
	else if (value == Py_False)
	{
		writeConstString("false");
	}
	else
	{
		PyObject* str = PyObject_Str(value);
		if (str)
		{
			const char* s = PyUnicode_AsUTF8(str);
			if (s) writeString(s);
		}
		else
		{
			// Ignore error and write dummy string (to keep script valid)
			PyErr_Clear();
			writeString("\"\"");
		}
	}
}


void MapWriter::formatAttributeValue(int key, PyObject* value)
{
	if (PyCallable_Check(value))
	{
		// PyObject* formatted = PyObject_CallOneArg(value, (PyObject*)binder_);
		//  works only for Python 3.9+
		PyObject* formatted = PyObject_CallFunctionObjArgs(value, (PyObject*)binder_, NULL);
		if (!formatted)
		{
			PyErr_Clear();
			writeString("\"\"");
			return;
		}
		int flags = PyMap::ATTR_FLAGS[key];
		if (flags & (PyMap::AttributeFlags::NUM_VALUE | PyMap::AttributeFlags::BOOL_VALUE))
		{
			if (flags & (PyMap::AttributeFlags::NUM_VALUE))
			{
				PyObject* numValue = PyFloat_FromString(formatted);
				if (numValue == NULL)
				{
					PyErr_Clear();
					// If the formatted string does not represent a valie number,
					// write the default value
					writeString(PyMap::ATTR_DEFAULTS[key]);
				}
				else
				{
					writeAttributeValue(numValue);
					Py_DECREF(numValue);
				}
			}
			else
			{
				// Boolean value
				const char* s = PyUnicode_AsUTF8(formatted);
				const char* boolString = "false";
				if (s == NULL)
				{
					PyErr_Clear();
				}
				else if (*s != 0 && strcmp(s, "False") != 0)
				{
					boolString = "true";
				}
				writeString(boolString);
			}
		}
		else
		{
			writeAttributeValue(formatted);
		}
		Py_DECREF(formatted);
		return;
	}
	else
	{
		writeAttributeValue(value);
	}
}

MapWriter::Schema::Schema()
{
	memset(this, 0, sizeof(*this));
}

// We don't need refcounting here because all objects are guaranteed
// to be live while we write the map data
void MapWriter::Schema::set(int key, PyObject* value)
{
	if (PyCallable_Check(value)) hasFormatters = true;
	switch (key)
	{
	case PyMap::TOOLTIP:
		tooltip = value;
		break;
	case PyMap::LINK:
		link = value;
		break;
	default:
		attributes[attrCount].key = key;
		attributes[attrCount].value = value;
		attrCount++;
		break;
	}
}

// TODO: element values could be null if add() failed due to invalid keywords!!!

void MapWriter::Schema::fill(const PyMap& map, const PyMap::Element* item)
{
	uint64_t specificElementAttributes = 0;
	const PyMap::ElementAttribute* p = item->attributes;
	const PyMap::ElementAttribute* end = p + item->attrCount;
	while (p < end)
	{
		int key = p->key;
		set(key, p->value);
		specificElementAttributes |= 1ULL << key;
		p++;
	}

	BitIterator<uint64_t> iter(map.globalElementAttributes);
	for (;;)
	{
		int key = iter.next();
		if (key < 0) break;
		if ((specificElementAttributes & (1ULL << key)) == 0)
		{
			set(key, map.attributes[key]);
		}
	}
}


void MapWriter::Schema::clear()
{
	attrCount = 0;
	tooltip = NULL;
	link = NULL;
	hasFormatters = false;
}


void MapWriter::initBinder()
{
	if (!binder_) binder_ = PyBinder::create();
}