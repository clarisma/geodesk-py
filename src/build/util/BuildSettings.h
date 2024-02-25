// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once
#include <cstdint>
#include <vector>
#include <string_view>
#ifdef GEODESK_PYTHON
#include <Python.h>
#endif
#include <common/validate/Validate.h>
#include "feature/ZoomLevels.h"

class BuildSettings
{
public:
	BuildSettings() :
		zoomLevels_(ZoomLevels::DEFAULT)
	{
	}

	static const uint32_t MAX_GLOBAL_STRING_CODE = (1 << 16) - 3;

	enum
	{
		AREA_TAGS,
		EXCLUDED_KEYS,
		ID_INDEXING,
		INDEXED_KEYS,
		KEY_INDEX_MIN_FEATURES,
		MAX_KEY_INDEXES,
		MAX_STRINGS,
		MAX_TILES,
		MIN_STRING_USAGE,
		MIN_TILE_DENSITY,
		PROPERTIES,
		RTREE_BRANCH_SIZE,
		SOURCE,
		THREADS,
		UPDATABLE,
		ZOOM_LEVELS,
	};

	#ifdef GEODESK_PYTHON

	typedef int (BuildSettings::* SetterMethod)(PyObject*);
	static const SetterMethod SETTER_METHODS[];

	int setOptions(PyObject* dict);
	int setAreaTags(PyObject* arg);
	int setExcludedKeys(PyObject* arg);
	int setIdIndexing(PyObject* arg);
	int setIndexedKeys(PyObject* arg);
	int setKeyIndexMinFeatures(PyObject* arg);
	int setMaxKeyIndexes(PyObject* arg);
	int setMaxStrings(PyObject* arg);
	int setMaxTiles(PyObject* arg);
	int setMinStringUsage(PyObject* arg);
	int setMinTileDensity(PyObject* arg);
	int setProperties(PyObject* arg);
	int setRTreeBranchSize(PyObject* arg);
	int setSource(PyObject* arg);
	int setThreads(PyObject* arg);
	int setUpdatable(PyObject* arg);
	int setZoomLevels(PyObject* arg);
	#endif

	const std::string& sourcePath() const { return sourcePath_; }
	uint32_t maxTiles() const { return maxTiles_; }
	uint32_t minTileDensity() const { return minTileDensity_; }
	int leafZoomLevel() const { return 12; }
	int threadCount() const { return threadCount_; }
	ZoomLevels zoomLevels() const { return zoomLevels_; }
	const std::vector<std::string_view>& indexedKeyStrings() const
	{
		return indexedKeyStrings_;
	}

	void setSource(const std::string_view path);

	void setMaxStrings(int64_t v)
	{
		if (v < 256) v = 256;
		Validate::max(v, MAX_GLOBAL_STRING_CODE + 1);
		maxStrings_ = v;
	}

	void setMaxTiles(int64_t v)
	{
		if (v < 1) v = 1;
		Validate::max(v, 8'000'000);
		maxTiles_ = v;
	}

	void setMinTileDensity(int64_t v)
	{
		if (v < 1) v = 1;
		Validate::max(v, 10'000'000);
		minTileDensity_ = v;
	}

	void setMinStringUsage(int64_t v)
	{
		if (v < 1) v = 1;
		Validate::max(v, 100'000'000);
		minStringUsage_ = v;
	}

	void setThreadCount(int64_t v)
	{
		if (v < 0) v = 0;
		threadCount_ = v;
	}

private:
	#ifdef GEODESK_PYTHON
	int addIndexedKey(PyObject* obj, int category);
	#endif
	void addIndexedKey(std::string_view key, int category);
	
	std::string sourcePath_;
	ZoomLevels zoomLevels_;
	uint32_t maxTiles_ = (1 << 16) - 1;
	uint32_t maxStrings_ = 300;
	uint32_t minStringUsage_ = 300;
	uint32_t minTileDensity_ = 25'000;
	int threadCount_ = 0;
	std::vector<std::string_view> indexedKeyStrings_;
	std::vector<uint8_t> indexedKeyCategories_;
};
