#include "TileLoader.h"
#include "query/TileIndexWalker.h"

TileLoader::TileLoader(FeatureStore *store) :
	store_(store),
	threadPool_(/* 1 */ std::thread::hardware_concurrency(), 0)
{
}


void TileLoader::load()
{
	TileIndexWalker tiw(store_->tileIndex(), store_->zoomLevels(), Box::ofWorld());
	while (tiw.next())
	{
		TileLoaderTask task(store_, tiw.currentTile(), tiw.currentTip());
		threadPool_.post(task);
	}
	threadPool_.awaitCompletion();
}


void TileLoaderTask::operator()()
{
	pointer pTile = store_->fetchTile(tip_);
	uint32_t size = pTile.getInt() & 0x3fff'ffff;
	uint8_t* pLoadedTile = new uint8_t[size];
	memcpy(pLoadedTile, pTile, size);
	delete[] pLoadedTile;
}

