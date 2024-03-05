#include "GolBuilder.h"
#include "Analyzer.h"
#include "Sorter.h"
#include "TileIndexBuilder.h"
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

	golPath_ = File::extension(golPath) != 0 ? golPath :
		std::string(golPath) + ".gol";
	workPath_ = golPath_.parent_path() / (golPath_.filename().string() + ".work");
	std::filesystem::create_directories(workPath_);

	analyze();
	prepare();
	sort();
	auto endTime = std::chrono::high_resolution_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime);
	printf("Built %s in %.3f seconds\n", golPath, duration.count() / 1e6);
}

void GolBuilder::analyze()
{
	Analyzer analyzer(this);
	analyzer.analyze(settings_.sourcePath().c_str());
	TileIndexBuilder tib(settings_);
	std::unique_ptr<const uint32_t[]> totalNodeCounts = analyzer.takeTotalNodeCounts();
	std::unique_ptr<const uint32_t[]> tileIndex(tib.build(totalNodeCounts.get()));
	delete totalNodeCounts.release();

	Console::msg("Building tile lookup...");
	tileCatalog_.build(tib.tileCount(), tileIndex.get(), settings_.zoomLevels());
	Console::msg("Tile lookup built.");
	stringCatalog_.build(settings_, analyzer.strings());
}

void GolBuilder::sort()
{
	Sorter sorter(this);
	sorter.sort(settings_.sourcePath().c_str());
}


void GolBuilder::openIndex(IndexFile& index, const char* name, int extraBits)
{
	int bits = 32 - Bits::countLeadingZerosInNonZero32(tileCatalog_.tileCount());
	int mode = File::OpenMode::CREATE | File::OpenMode::READ | File::OpenMode::WRITE;
	index.bits(bits + extraBits);
	index.open((workPath_ / name).string().c_str(), mode);
}

void GolBuilder::prepare()
{
	openIndex(featureIndexes_[0], "nodes.idx", 0);
	openIndex(featureIndexes_[1], "ways.idx", 2);
	openIndex(featureIndexes_[2], "relations.idx", 2);
	featurePiles_.create((workPath_ / "features.bin").string().c_str(), 
		tileCatalog_.tileCount(), 64 * 1024);
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

