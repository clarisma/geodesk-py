#include "GolBuilder.h"
#include <common/sys/SystemInfo.h>
#include "build/analyze/Analyzer.h"
#include "build/analyze/TileIndexBuilder.h"
#include "build/sort/Sorter.h"
#include "build/sort/Validator.h"
#include "build/compile/Compiler.h"
#ifdef GEODESK_PYTHON
#include "python/util/util.h"
#endif

GolBuilder::GolBuilder() :
	threadCount_(0),
	workCompleted_(0)
{
}

// Resources we need:
// - A lookup from coordinates to tiles



void GolBuilder::build(const char* golPath)
{
	int cores = std::thread::hardware_concurrency();
	threadCount_ = settings_.threadCount();
	if (threadCount_ == 0)
	{
		threadCount_ = cores;
	}
	else if (threadCount_ > 4 * cores)
	{
		threadCount_ = 4 * cores;
	}

	/*
	#ifdef _DEBUG
	threadCount_ = 1;
	#endif // _DEBUG
	*/

	auto startTime = std::chrono::high_resolution_clock::now();

	console_.start("Analyzing...");
	// SystemInfo sysinfo;
	calculateWork();

	golPath_ = File::extension(golPath) != 0 ? golPath :
		std::string(golPath) + ".gol";
	workPath_ = golPath_.parent_path() / (golPath_.filename().string() + ".work");
	std::filesystem::create_directories(workPath_);

	analyze();
	prepare();
	sort();
	validate();
	compile();
	auto endTime = std::chrono::high_resolution_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime);
	console_.log("Done.");
}

void GolBuilder::analyze()
{
	Analyzer analyzer(this);
	analyzer.analyze(settings_.sourcePath().c_str());
	stats_ = analyzer.osmStats();

	console_.setTask("Preparing indexes...");

	TileIndexBuilder tib(settings_);
	std::unique_ptr<const uint32_t[]> totalNodeCounts = analyzer.takeTotalNodeCounts();
	tib.build(std::move(totalNodeCounts));
	std::unique_ptr<const uint32_t[]> tileIndex = tib.takeTileIndex();
	tileSizeEstimates_ = tib.takeTileSizeEstimates();
	
	Console::msg("Building tile lookup...");
	tileCatalog_.build(tib);
	tileCatalog_.write(workPath_ / "tile-catalog.txt");
	Console::msg("Tile lookup built.");
	stringCatalog_.build(settings_, analyzer.strings());
}


void GolBuilder::createIndex(MappedIndex& index, const char* name, int64_t maxId, int extraBits)
{
	int bits = 32 - Bits::countLeadingZerosInNonZero32(tileCatalog_.tileCount());
		// The above is correct; if we have 512 tiles, we need to store 513
		// distinct values (pile number starts at 1, 0 = "missing")
	index.create((workPath_ / name).string().c_str(), maxId, bits + extraBits);
}

void GolBuilder::prepare()
{
	createIndex(featureIndexes_[0], "nodes.idx", stats_.maxNodeId, 0);
	createIndex(featureIndexes_[1], "ways.idx", stats_.maxWayId, 2);
	createIndex(featureIndexes_[2], "relations.idx", stats_.maxRelationId, 2);

	// TODO: Decide whether tileSizeEstimates_ is 0-based or 1-based

	int tileCount = tileCatalog_.tileCount();
	featurePiles_.create((workPath_ / "features.bin").string().c_str(), 
		tileCount, 64 * 1024, tileSizeEstimates_[0]);

	for (int i = 1; i <= tileCount; i++)
	{
		featurePiles_.preallocate(i, tileSizeEstimates_[i]);
	}
}


void GolBuilder::sort()
{
	Sorter sorter(this);
	sorter.sort(settings_.sourcePath().c_str());
}

void GolBuilder::validate()
{
	Validator::Validator validator(this);
	validator.validate();
}

void GolBuilder::compile()
{
	Compiler compiler(this);
	compiler.compile();
}

#ifdef GEODESK_PYTHON

PyObject* GolBuilder::build(PyObject* args, PyObject* kwds)
{
	GolBuilder builder;
	PyObject* arg = PyTuple_GetItem(args, 0);
	const char* golFile = PyUnicode_AsUTF8(arg);
	if (!golFile) return NULL;
	if (builder.setOptions(kwds) < 0) return NULL;
	builder.build(golFile);
	Py_RETURN_NONE; // TODO
}

#endif

void GolBuilder::calculateWork()
{
	workPerPhase_[ANALYZE]  = 10.0;
	workPerPhase_[SORT]     = 40.0;
	workPerPhase_[VALIDATE] = 20.0;
	workPerPhase_[COMPILE]  = 30.0;
}

