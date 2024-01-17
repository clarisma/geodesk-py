#include "BuildCommand.h"
#include <chrono>
#include "build/Analyzer.h"

BuildCommand::BuildCommand()
{

}

void BuildCommand::setParam(int number, std::string_view value)
{
	switch(number)
	{ 
	case 0:
		return;
	case 1:
		golPath_ = GolCommand::golPath(value);
		return;
	case 2:
		sourcePath_ = GolCommand::pathWithExtension(value, ".osm.pbf");
		return;
	}
	// TODO: Superfluous arguments
}

void BuildCommand::setOption(std::string_view name, std::string_view value)
{

}

void BuildCommand::run(char* argv[])
{
	CliCommand::run(argv);
	int threads = std::thread::hardware_concurrency();
	auto startTime = std::chrono::high_resolution_clock::now();
	Analyzer analyzer(threads);
	analyzer.analyze(sourcePath_.c_str());
	auto endTime = std::chrono::high_resolution_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime);
	printf("Built %s in %.3f seconds\n", golPath_.c_str(), duration.count() / 1e6);
}
