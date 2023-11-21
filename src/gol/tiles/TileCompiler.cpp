#include "TileCompiler.h"
#include "query/TileIndexWalker.h"
#include "IndexSettings.h"
#include "Layout.h"
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
	outFile_= std::ofstream("c:\\geodesk\\debug\\new-planet.bin", std::ios::binary);
	TileIndexWalker tiw(store_->tileIndex(), store_->zoomLevels(), Box::ofWorld());
	while (tiw.next())
	{
		TileCompilerTask task(this, tiw.currentTile(), tiw.currentTip());
		workers_.post(task);
	}
	workers_.awaitCompletion();
	writer_.awaitCompletion();
	outFile_.close();
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

	Layout layout(tile);
	indexer.place(layout);
	layout.placeBodies();
	uint8_t* newTileData = tile.write(layout);
	compiler_->writer_.post(TileWriterTask(compiler_, tip_, newTileData, layout.size()));
	/*
	uint8_t* newTileData = new uint8_t[layout.size()];
	TElement* elem = layout.first();
	do
	{
		switch (elem->type())
		{
		case TElement::Type::FEATURE:
			reinterpret_cast<TFeature*>(elem)->write(&tile, )
		}
	}
	*/
	/*
	memcpy(pLoadedTile, pTile, size);
	delete[] pLoadedTile;
	*/
}


void TileWriterTask::operator()()
{
	compiler_->outFile_.write(reinterpret_cast<const char*>(data_), size_);
	delete[] data_;
}
