#include "TileCompiler.h"
#include "query/TileIndexWalker.h"
#include "IndexSettings.h"
#include "TTile.h"
#include "TIndex.h"

TileCompiler::TileCompiler(FeatureStore* store) :
	store_(store),
#ifdef _DEBUG
	workers_(1, 0),
#else
	workers_(std::thread::hardware_concurrency(), 0),
#endif
	writer_(1, 8)
{
}


void TileCompiler::compile()
{
	TileIndexWalker tiw(store_->tileIndex(), store_->zoomLevels(), Box::ofWorld());
	while (tiw.next())
	{
		TileCompilerTask task(this, tiw.currentTile(), tiw.currentTip());
		workers_.post(task);
	}
	workers_.awaitCompletion();
	writer_.awaitCompletion();
}


void TileCompilerTask::operator()()
{
	FeatureStore* store = compiler_->store_;
	pointer pTile = store->fetchTile(tip_);
	// uint32_t size = pTile.getInt() & 0x3fff'ffff;
	// uint8_t* pLoadedTile = new uint8_t[size];

	TTile tile(tile_);
	store->prefetchBlob(pTile);
	tile.readTile(pTile);

	IndexSettings indexSettings(store, 8, 8, 300); // TODO
	Indexer indexer(tile, indexSettings);
	indexer.addFeatures(tile.features());
	indexer.build();
	/*
	memcpy(pLoadedTile, pTile, size);
	delete[] pLoadedTile;
	*/
}


void TileWriterTask::operator()()
{
}
