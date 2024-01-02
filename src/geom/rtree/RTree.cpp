// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#include "RTree.h"

/*

template <typename IT>
template <typename QT>
bool RTree<IT>::searchLeaf(const Query<QT>& query, const Node* p)
{
	for (;;p++)
	{
		bool found = false;
		int endFlag = p->endFlag();
		if (query.checkIntersection(p))
		{
			found = (*query.func)(p, query.closure);
		}
		if ((static_cast<int>(found) | endFlag) != 0) return found;	// bitwise | intended
	}
}

template <typename IT>
template <typename QT>
bool RTree<IT>::searchTrunk(const Query<QT>& query, const Node* p)
{
	for (;; p++)
	{
		bool found = false;
		void* pItem = p->childOrItem;
		uintptr_t intPtr = reinterpret_cast<uintptr_t>(pItem);
		int endFlag = intPtr & 1;
		int leafFlag = intPtr & 2;
		if (query.checkIntersection(p))
		{
			const Node* pChild = reinterpret_cast<const Node*>(intPtr & ~3);
			if (leafFlag)
			{
				found = searchLeaf(query, pChild);
			}
			else
			{
				found = searchTrunk(query, pChild);
			}
		}
		if ((static_cast<int>(found) | endFlag) != 0) return found;	// bitwise | intended
	}
}


*/