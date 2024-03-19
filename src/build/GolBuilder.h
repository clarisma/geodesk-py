// Copyright (c) 2023 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once
#include <filesystem>
#include <string_view>
#include <common/cli/Console.h>
#include <common/store/PileFile.h>
#include "build/analyze/OsmStatistics.h"
#include "build/util/BuildSettings.h"
#include "build/util/MappedIndex.h"
#include "build/util/StringCatalog.h"
#include "build/util/TileCatalog.h"


class GolBuilder
{
public:
	GolBuilder();

	enum Phase { ANALYZE, SORT, VALIDATE, COMPILE };

	#ifdef GEODESK_PYTHON
	static PyObject* build(PyObject* args, PyObject* kwds);
	int setOptions(PyObject* dict)
	{
		return settings_.setOptions(dict);
	}
	#endif

	void build(const char* golPath);
	Console& console() { return console_; }
	const BuildSettings& settings() const { return settings_; }
	int threadCount() const { return threadCount_; }
	const StringCatalog& stringCatalog() const { return stringCatalog_; }
	const TileCatalog& tileCatalog() const { return tileCatalog_; }
	MappedIndex& featureIndex(int index) 
	{ 
		assert(index >= 0 && index <= 2);
		return featureIndexes_[index]; 
	}
	PileFile& featurePiles() { return featurePiles_; }
	double phaseWork(int phase) const { return workPerPhase_[phase]; }
	void progress(double work)
	{
		workCompleted_ += work;
		console_.setProgress(static_cast<int>(workCompleted_));
	}

private:
	void analyze();
	void prepare();
	void sort();
	void validate();
	void compile();

	void calculateWork();
	void createIndex(MappedIndex& index, const char* name, int64_t maxId, int extraBits);

	Console console_;
	BuildSettings settings_;
	std::filesystem::path golPath_;
	std::filesystem::path workPath_;
	StringCatalog stringCatalog_;
	TileCatalog tileCatalog_;
	std::unique_ptr<const uint32_t[]> tileSizeEstimates_;
	MappedIndex featureIndexes_[3];
	PileFile featurePiles_;
	OsmStatistics stats_;
	int threadCount_;
	double workPerPhase_[4];
	double workCompleted_;
};