// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#include "HilbertTreeBuilder.h"
#include <algorithm>
#include <math.h>
#include <utility>
#include "hilbert.h"

/*
 *
 * This class is based on flatbush (https://github.com/mourner/flatbush)
 * by Volodymyr Agafonkin. The original work is licensed as follows:
 *
 * ISC License
 *
 * Copyright (c) 2018, Volodymyr Agafonkin
 *
 * Permission to use, copy, modify, and/or distribute this software for any purpose
 * with or without fee is hereby granted, provided that the above copyright notice
 * and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH
 * REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND
 * FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
 * INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
 * OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF
 * THIS SOFTWARE.
 *
 * (https://github.com/mourner/flatbush/blob/master/LICENSE)
 */

HilbertTreeBuilder::HilbertTreeBuilder(Arena* arena) :
	arena_(arena ? *arena : ownArena_)
{

}

// TODO: what happens if tree ie empty??
// Not allowed --> enforce in PyRTree
// or could have an empty-box root, with a world-box child
// --> check if it matches anything

const HilbertTreeBuilder::Node* HilbertTreeBuilder::buildNodes(
	const BoundedItem* items, size_t itemCount, int maxItemsPerNode,
	Box totalBounds)
{
	if(totalBounds.isEmpty())
	{
		for (const BoundedItem* p = items; p < items + itemCount; p++)
		{
			totalBounds.expandToIncludeSimple(p->bounds);
		}
	}

	HilbertItem* hilbertItems = buildHilbertIndex(items, itemCount, totalBounds);

	size_t totalNodeCount = calculateTotalNodeCount(itemCount, maxItemsPerNode);

	Node* tree = new Node[totalNodeCount];
	Node* pChildEnd = tree + totalNodeCount;
	Node* pChildStart = tree + totalNodeCount - itemCount;
	Node* p = pChildStart;
	HilbertItem* pItem = hilbertItems;
	do
	{
		p->init(pItem->second->bounds, pItem->second->item, 0);
		p++;
		pItem++;
	} 
	while (p < pChildEnd);

	size_t childCount = itemCount;
	int flags = Node::Flags::LEAF;

	for(;;)
	{
		// For each level
		assert(childCount == pChildEnd - pChildStart);
		size_t parentCount = (childCount + maxItemsPerNode - 1) / maxItemsPerNode;
		Node* pChild = pChildStart;

		// Place parents before children
		Node* pParentStart = pChildStart - parentCount;
		Node* pParent = pParentStart;
		do
		{
			Box parentBounds;
			Node* pFirstChild = pChild;
			Node* pNextChild = pChild + maxItemsPerNode;
			pNextChild = std::min(pNextChild, pChildEnd);
			do
			{
				parentBounds.expandToIncludeSimple(pChild->bounds);
				pChild++;
			} 
			while (pChild < pNextChild);
			(pChild - 1)->markLast();
			pParent->init(parentBounds, pFirstChild, flags);
			pParent++;
			parentBounds.setEmpty();
		} 
		while (pChild < pChildEnd);		
		flags = 0;
		pChildEnd = pChildStart;
		pChildStart = pParentStart;
		childCount = parentCount;
		if (parentCount == 1) break;
	}
	tree->markLast();				// root is a single node
		// (We marked the last child when we processed the parents, 
		//  but as root has no parent, we need to mark it explicitly here)
	assert(pChildStart == tree);
	return tree;
}


HilbertTreeBuilder::HilbertItem* HilbertTreeBuilder::buildHilbertIndex(
	const BoundedItem* items, size_t itemCount, const Box& totalBounds)
{
	int64_t totalWidth = totalBounds.widthSimple();
	int64_t totalHeight = totalBounds.height();

	HilbertItem* hilbertItems = arena_.allocArray<HilbertItem>(itemCount);
	HilbertItem* p = hilbertItems;

	for (const BoundedItem* pBounded = items; pBounded < items + itemCount; pBounded++)
	{
		const Box& b = pBounded->bounds;
		int64_t relX = (
			static_cast<int64_t>(b.minX()) + 
			static_cast<int64_t>(b.maxX()))
			/ 2 - totalBounds.minX();
		int64_t relY = (
			static_cast<int64_t>(b.minY()) +
			static_cast<int64_t>(b.maxY()))
			/ 2 - totalBounds.minY();

		int32_t hilbertX = (static_cast<int64_t>(hilbert::MAX_COORDINATE) * relX) / totalWidth;
		int32_t hilbertY = (static_cast<int64_t>(hilbert::MAX_COORDINATE) * relY) / totalHeight;
		p->first = hilbert::calculateHilbertDistance(hilbertX, hilbertY);
		p->second = pBounded;
		p++;
	}
	std::sort(hilbertItems, p);
	return hilbertItems;
}

size_t HilbertTreeBuilder::calculateTotalNodeCount(size_t itemCount, int maxItemsPerNode)
{
	size_t nodeCount = itemCount;
	size_t n = itemCount;
	for(;;)
	{
		n = (n + maxItemsPerNode - 1) / maxItemsPerNode;
		nodeCount += n;
		if (n == 1) break;
	} 
	return nodeCount;
}
