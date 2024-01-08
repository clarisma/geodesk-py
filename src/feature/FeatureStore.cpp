// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#include "FeatureStore.h"

#include <filesystem>
// #include <unicodeobject.h>
#include <common/util/log.h>
#include <common/util/PbfDecoder.h>
// #include "python/query/PyFeatures.h"

// TODO: std::thread::hardware_concurrency() can return 0, so use
// a default value in that case (e.g. 4 threads)

std::unordered_map<std::string, FeatureStore*> FeatureStore::openStores_;

FeatureStore::FeatureStore()
  : refcount_(1),
	matchers_(this),	// TODO: this not initialized yet!
	executor_(/* 1 */ std::thread::hardware_concurrency(), 0)  // TODO: disabled for testing
{
}

FeatureStore* FeatureStore::openSingle(std::string_view relativeFileName)
{
	std::filesystem::path path;
	try
	{
		path = std::filesystem::canonical(
			(*File::extension(relativeFileName) != 0) ? relativeFileName :
			std::string(relativeFileName) + ".gol");
	}
	catch (const std::filesystem::filesystem_error&)
	{
		throw FileNotFoundException(std::string(relativeFileName));
	}
	std::string fileName = path.string();

	FeatureStore* store;
	auto it = openStores_.find(fileName);
	if (it != openStores_.end()) 
	{
		store = it->second;
		store->addref();
		return store;
	}
	store = new FeatureStore();
	try
	{
		store->open(fileName.data());
		openStores_[fileName] = store;
		return store;
	}
	catch (...)
	{
		delete store;
		throw;
	}
}

void FeatureStore::initialize()
{
	strings_.create(getPointer(STRING_TABLE_PTR_OFS));
	zoomLevels_ = pointer(mainMapping()).getUnsignedInt(ZOOM_LEVELS_OFS);
	readIndexSchema();
}

FeatureStore::~FeatureStore()
{
	LOG("Destroying FeatureStore...");
	LOG("Destroyed FeatureStore.");
	openStores_.erase(fileName());
}

pointer FeatureStore::fetchTile(Tip tip)
{
	uint32_t pageEntry = (tileIndex() + (tip << 2)).getUnsignedInt();
	// Bit 0 is a flag bit (page vs. child pointer)
	// TODO: load tiles

	return pagePointer(pageEntry >> 1);
}



void FeatureStore::readIndexSchema()
{
	pointer p = getPointer(INDEX_SCHEMA_PTR_OFS);
	int32_t count = p.getInt();
	keysToCategories_.reserve(count);
	for (int i = 0; i < count; i++)
	{
		p += 4;
		keysToCategories_.insert({ p.getUnsignedShort(), p.getUnsignedShort(2) });
	}
}

int FeatureStore::getIndexCategory(int keyCode) const
{
	auto it = keysToCategories_.find(keyCode);
	if (it != keysToCategories_.end())
	{
		return it->second;
	}
	else
	{
		return 0;
	}
}

const MatcherHolder* FeatureStore::getMatcher(const char* query)
{
	return matchers_.getMatcher(query);
}

#ifdef GEODESK_PYTHON

/*
PyObject* FeatureStore::emptyString()
{
	// TODO
	Py_RETURN_NONE;
}
*/

PyObject* FeatureStore::emptyTags()
{
	// TODO
	Py_RETURN_NONE;
}

#endif


