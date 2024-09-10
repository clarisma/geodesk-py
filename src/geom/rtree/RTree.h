// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once
#include <cstdint>
#include "geom/Box.h"

struct BoundedItem
{
	Box bounds;
	void* item;
};

// Root branch must always be a trunk, even if there is only a single child branch

template <typename IT>
class RTree
{
public:
	struct Node
	{
		Box bounds;

		enum Flags
		{
			LAST = 1,
			LEAF = 2
		};

		void init(const Box& b, const void* p, int flags)
		{
			bounds = b;
			childOrItem = reinterpret_cast<Node*>(
				reinterpret_cast<uintptr_t>(p) | flags);
		}

		void markLast()
		{
			childOrItem = reinterpret_cast<Node*>(
				reinterpret_cast<uintptr_t>(childOrItem) | LAST);
		}

		IT* item() const { return reinterpret_cast<IT*>(reinterpret_cast<uintptr_t>(childOrItem) & ~1); }
		int endFlag() const { return reinterpret_cast<uintptr_t>(childOrItem) & 1; }

	private:
		Node* childOrItem;

		friend class RTree;
	};

	template <typename QT>
	using SearchFunction = bool (*)(const Node* node, QT closure);

	RTree() : root_(nullptr) {}		
	~RTree() { if (root_) delete[] root_; }		// root_ is an array

	RTree(const Node* root) : root_(root) {}

	// Disable copy semantics as RTree owns resources
	RTree(const RTree& other) = delete;
	RTree& operator=(const RTree& other) = delete;
	RTree(RTree&& other) noexcept : root_(other.root_)
	{
		other.root_ = nullptr; // Prevent other from deallocating the memory
	}

	RTree& operator=(RTree&& other) noexcept
	{
		if (this != &other && root_) delete root_; // Release currently owned memory (if any)
		root_ = other.root_;
		other.root_ = nullptr;
		return *this;
	}

	const Node* root() const { return root_; }

	template <typename QT>
	bool search(const Box& box, SearchFunction<QT> func, QT closure) const
	{
		Query<QT> query(box, func, closure);
		return searchTrunk(query, root_);
	}


private:
	template <typename QT>
	class Query
	{
	public:
		Query(const Box& box, SearchFunction<QT> func, QT closure) :
			bounds_(box),
			func_(func),
			closure_(closure)
		{
		}

		inline bool checkIntersection(const Node* node) const
		{
			return bounds_.intersects(node->bounds);
		}

	// private:
		Box bounds_;
		SearchFunction<QT> func_;
		QT closure_;
	};

	template <typename QT>
	static bool searchLeaf(const Query<QT>& query, const Node* p)
	{
		for (;; p++)
		{
			bool found = false;
			int endFlag = p->endFlag();
			if (query.checkIntersection(p))
			{
				found = (*query.func_)(p, query.closure_);
			}
			if ((static_cast<int>(found) | endFlag) != 0) return found;	// bitwise | intended
		}
	}

	template <typename QT>
	static bool searchTrunk(const Query<QT>& query, const Node* p)
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
		
	const Node* root_;
};
