// Copyright (c) 2023 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include "RTree.h"
#include <common/alloc/Arena.h>

/*
class RTreeBuilder
{
public:
	RTreeBuilder(Arena& arena, int nodeSize) : arena_(arena), nodeSize_(nodeSize) {}
	size_t storageSize() const;
	template <typename IT>
	void build(RTree<IT>::Node* pNodes, int itemCount);

private:
	Arena& arena_;
	int nodeSize_;
};
*/