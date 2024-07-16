// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#include <chrono>
#include "feature/FeatureStore.h"
#include "query/Query.h"
#include "BuildCommand.h"
#include "GolTool.h"

/*
void testPileFile()
{
	File file;
	file.open("c:\\geodesk\\tests\\trash.bin",
		File::OpenMode::READ | File::OpenMode::WRITE | File::OpenMode::CREATE);
	size_t size = 5ULL * 1024 * 1024 * 1024;
	file.setSize(size);
	file.zeroFill(0, size);
	file.makeSparse();
	file.setSize(size * 2);
	file.close();
}
*/

int main(int argc, char* argv[])
{
	GolTool app;
	return app.run(argv);
}
	// testPileFile();

	//BuildCommand cmd;
	//cmd.run(argv);

	/*
	FeatureStore store;
	store.open(argv[2]);

	for (int i = 0; i < 10; i++)
	{
		auto start = std::chrono::high_resolution_clock::now();
		const MatcherHolder* matcher = store.getMatcher(argv[3]);
		Box bounds(Box::ofWorld());
		Query query(&store, bounds, matcher->acceptedTypes(), matcher, nullptr);

		uint64_t count = 0;
		for (;;)
		{
			pointer pFeature = query.next();
			if (!pFeature) break;
			count++;
		}
		matcher->release();

		auto stop = std::chrono::high_resolution_clock::now();
		auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);

		printf("%lld features found in %.3f seconds\n", count, duration.count() / 1e6);
	}
	*/
// }

