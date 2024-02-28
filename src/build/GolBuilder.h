// Copyright (c) 2023 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once
#include <filesystem>
#include <string_view>
#include <common/store/IndexFile.h>
#include "build/util/BuildSettings.h"
#include "build/util/StringCatalog.h"
#include "build/util/TileCatalog.h"


class GolBuilder
{
public:
	GolBuilder();

	#ifdef GEODESK_PYTHON
	static PyObject* build(PyObject* args, PyObject* kwds);
	int setOptions(PyObject* dict)
	{
		return settings_.setOptions(dict);
	}
	#endif

	void build(const char* golPath);
	const BuildSettings& settings() const { return settings_; }
	int threadCount() const { return threadCount_; }
	const StringCatalog& stringCatalog() const { return stringCatalog_; }
	const TileCatalog& tileCatalog() const { return tileCatalog_; }
	IndexFile& nodeIndex() { return nodeIndex_; }
	IndexFile& wayIndex() { return wayIndex_; }
	IndexFile& relationIndex() { return relationIndex_; }

private:
	void analyze();
	void sort();
	void validate();
	void compile();

	void openIndexes();
	void openIndex(IndexFile& index, const char* name, int extraBits);

	BuildSettings settings_;
	std::filesystem::path golPath_;
	std::filesystem::path workPath_;
	StringCatalog stringCatalog_;
	TileCatalog tileCatalog_;
	IndexFile nodeIndex_;
	IndexFile wayIndex_;
	IndexFile relationIndex_;
	int threadCount_;
};
