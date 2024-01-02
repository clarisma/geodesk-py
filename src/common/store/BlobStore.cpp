// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#include "BlobStore.h"

uint64_t BlobStore::getLocalCreationTimestamp() const 
{
	return pointer(mainMapping()).getUnsignedLong(LOCAL_CREATION_TIMESTAMP_OFS);
}

uint64_t BlobStore::getTrueSize() const 
{
	int totalPages = pointer(mainMapping()).getInt(TOTAL_PAGES_OFS);
	// TODO: totalPages should really be unsigned, but we treat it as int
	// to stay compatible with the Java implementation
	// TODO: A FeatureStore cannot have more than 2^31 pages because 
	// of the marker bit in the tile index
	return static_cast<uint64_t>(totalPages) << pageSizeShift_;
}

void BlobStore::verifyHeader() const 
{
	if (pointer(mainMapping()).getUnsignedInt() != MAGIC)
	{
		error("Not a BlobStore file");
	}
}


void BlobStore::createStore()
{
	// TODO
}


void BlobStore::initialize()
{
	// Do nothing (TOOD: may change)
}


