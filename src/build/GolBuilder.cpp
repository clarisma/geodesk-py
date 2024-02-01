#include "GolBuilder.h"
#include "Analyzer.h"
#ifdef GEODESK_PYTHON
#include "python/util/util.h"
#endif

GolBuilder::GolBuilder()
{

}


void GolBuilder::build(const char* golPath)
{
	int cores = std::thread::hardware_concurrency();
	int threads = settings_.threadCount();
	if (threads == 0)
	{
		threads = cores;
	}
	else if (threads > 4 * cores)
	{
		threads = 4 * cores;
	}
	auto startTime = std::chrono::high_resolution_clock::now();
	Analyzer analyzer(threads);
	analyzer.analyze(settings_.sourcePath().c_str());
	auto endTime = std::chrono::high_resolution_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime);
	printf("Built %s in %.3f seconds\n", golPath, duration.count() / 1e6);
}

#ifdef GEODESK_PYTHON

PyObject* GolBuilder::build(PyObject* args, PyObject* kwds)
{
	GolBuilder builder;
	PyObject* arg = PyTuple_GetItem(args, 0);
	const char* golFile = PyUnicode_AsUTF8(arg);
	if (!golFile) return NULL;
	builder.setOptions(kwds);
	builder.build(golFile);
	Py_RETURN_NONE; // TODO
}

#endif

