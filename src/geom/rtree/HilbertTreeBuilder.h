// Copyright (c) 2023 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include <vector>
#include "RTree.h"
#include "geom/Box.h"
#include <common/alloc/Arena.h>

class HilbertTreeBuilder
{
public:
	typedef RTree<const void>::Node Node;

	HilbertTreeBuilder(Arena* arena);
	const Node* buildNodes(const BoundedItem* items, size_t itemCount, 
		int maxItemsPerNode, Box totalBounds);

	template <typename IT>
	RTree<IT> build(const BoundedItem* items, size_t itemCount,
		int maxItemsPerNode, Box totalBounds)
	{
		return RTree<IT>(
			reinterpret_cast<const typename RTree<IT>::Node*>(
				buildNodes(items, itemCount, maxItemsPerNode, totalBounds)));
	}

private:
	typedef std::pair<uint32_t, const BoundedItem*> HilbertItem;
	
	static size_t calculateTotalNodeCount(size_t itemCount, int maxItemsPerNode);
	HilbertItem* buildHilbertIndex(const BoundedItem* items, size_t itemCount, const Box& totalBounds);

	Arena& arena_;
	Arena ownArena_;
};
