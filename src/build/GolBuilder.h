// Copyright (c) 2023 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once
#include <filesystem>
#include <string_view>
#include <common/store/IndexFile.h>
#include <common/store/PileFile.h>
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
	IndexFile& featureIndex(int index) 
	{ 
		assert(index >= 0 && index <= 2);
		return featureIndexes_[index]; 
	}
	IndexFile& nodeIndex() { return featureIndex(0); }
	PileFile& featurePiles() { return featurePiles_; }

private:
	void analyze();
	void prepare();
	void sort();
	void validate();
	void compile();

	void openIndex(IndexFile& index, const char* name, int extraBits);

	BuildSettings settings_;
	std::filesystem::path golPath_;
	std::filesystem::path workPath_;
	StringCatalog stringCatalog_;
	TileCatalog tileCatalog_;
	IndexFile featureIndexes_[3];
	PileFile featurePiles_;
	int threadCount_;
};