#include "GolBuilder.h"
#include "Analyzer.h"
#include "TileIndexBuilder.h"
#include "build/util/TileLookupBuilder.h"
#ifdef GEODESK_PYTHON
#include "python/util/util.h"
#endif

GolBuilder::GolBuilder() :
	threadCount_(0)
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

	#ifdef _DEBUG
	threadCount_ = 1;
	#endif // _DEBUG

	auto startTime = std::chrono::high_resolution_clock::now();
	analyze();
	auto endTime = std::chrono::high_resolution_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime);
	printf("Built %s in %.3f seconds\n", golPath, duration.count() / 1e6);
}

void GolBuilder::analyze()
{
	Analyzer analyzer(settings_, threadCount_);
	analyzer.analyze(settings_.sourcePath().c_str());
	TileIndexBuilder tib(settings_);
	std::unique_ptr<const uint32_t[]> totalNodeCounts = analyzer.takeTotalNodeCounts();
	std::unique_ptr<const uint32_t[]> tileIndex(tib.build(totalNodeCounts.get()));
	delete totalNodeCounts.release();

	TileLookupBuilder tileLookupBuilder(tileIndex.get(), settings_.zoomLevels());
	tileLookupBuilder.build();

	stringManager_.build(settings_, analyzer.strings());
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

