// Copyright (c) 2023 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once
#include <string_view>
#include "build/util/BuildSettings.h"
#include "build/util/StringManager.h"
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
	const StringManager& stringManager() const { return stringManager_; }
	const TileCatalog& tileCatalog() const { return tileCatalog_; }

private:
	void analyze();
	void sort();
	void validate();
	void compile();

	BuildSettings settings_;
	StringManager stringManager_;
	TileCatalog tileCatalog_;
	int threadCount_;
};
