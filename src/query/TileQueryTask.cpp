// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#include "TileQueryTask.h"
#include "feature/Feature.h"
#include "feature/types.h"
#include "Query.h"

QueryResultsHeader QueryResults::EMPTY_HEADER = { EMPTY, nullptr, DEFAULT_BUCKET_SIZE };
QueryResults* const QueryResults::EMPTY = reinterpret_cast<QueryResults*>(&EMPTY_HEADER);

// TODO: perform type check prior to matcher

	// Check for acceptable type (way, relation, member, way-node, etc.)
	// The following is valid for Java, check for C++:
	// (No need for AND with 0x1f, as int-shift only considers lower 5 bits)
	// if (((1 << (flags >> 1)) & acceptedTypes) == 0) break;

void TileQueryTask::operator()()
{
	uint32_t tip = tipAndFlags_ >> 8;
	pTile_ = query_->store()->fetchTile(tip);
	uint32_t types = query_->types();

	// LOG("Scanning tile %06X", tip);

	if (types & FeatureTypes::NODES) searchNodeIndexes();
	if (types & FeatureTypes::NONAREA_WAYS) searchIndexes(FeatureIndexType::WAYS);
	if (types & FeatureTypes::AREAS) searchIndexes(FeatureIndexType::AREAS);
	if (types & FeatureTypes::NONAREA_RELATIONS) searchIndexes(FeatureIndexType::RELATIONS);
	query_->offer(results_);
}

void TileQueryTask::searchNodeIndexes()
{
	const MatcherHolder* matcher = query_->matcher();
	pointer ppRoot = pTile_ + 8;
	int32_t ptr = ppRoot.getInt();
	if (ptr == 0) return;
	if ((ptr & 1) == 0)
	{
		// method_ = matcher->method(FeatureIndexType::NODES);
		searchNodeRoot(ppRoot);
		return;
	}
	
	pointer p = ppRoot + (ptr ^ 1);
	for (;;)
	{
		int32_t last = p.getInt() & 1;
		int32_t keys = p.getInt(4);
		if (matcher->acceptIndex(FeatureIndexType::NODES, keys))
		{
			searchNodeRoot(p);
		}
		if (last != 0) break;
		p += 8;
	}
}

void TileQueryTask::searchNodeRoot(pointer ppRoot)
{
	int32_t ptr = ppRoot.getInt();
	if (ptr)
	{
		pointer p = ppRoot + (ptr & 0xffff'fffc);
		if (ptr & 2)
		{
			searchNodeLeaf(p);
		}
		else
		{
			searchNodeBranch(p);
		}
	}
}

void TileQueryTask::searchNodeBranch(pointer p)
{
	// LOG("Searching branch at %016X", p);
	Box box = query_->bounds();
	for (;;)
	{
		int32_t ptr = p.getInt();
		int32_t last = ptr & 1;
		if (box.intersects(*reinterpret_cast<const Box*>((const uint8_t *)p + 4)))
		{
			pointer pChild = p + (ptr & 0xffff'fffc);
			if (ptr & 2)
			{
				searchNodeLeaf(pChild);
			}
			else
			{
				searchNodeBranch(pChild);
			}
		}
		if (last != 0) break;
		p += 20;
	}
}

void TileQueryTask::searchNodeLeaf(pointer p)
{
	// LOG("Searching leaf at %016X", p);
	Box box = query_->bounds();
	FeatureTypes acceptedTypes = query_->types();
	const Matcher& matcher = query_->matcher()->mainMatcher();

	for (;;)
	{
		int32_t flags = p.getInt(8);
		if (box.contains(p.getInt(), p.getInt(4)))
		{
			if (acceptedTypes.acceptFlags(flags))
			{
				pointer pFeature = p + 8;
				if (matcher.accept(pFeature))
				{
					const Filter* filter = query_->filter();
					if (filter == nullptr || filter->accept(query_->store(),
						FeatureRef(pFeature), fastFilterHint_))
					{
						// LOG("Found node/%llu", Feature::id(pFeature));
						addResult(pFeature - pTile_);	// TODO
					}
				}
			}
		}
		if (flags & 1) break;
		p += 20 + (flags & 4);	
		// If Node is member of relation (flag bit 2), add
		// extra 4 bytes for the relation table pointer
	}
}



void TileQueryTask::searchIndexes(FeatureIndexType indexType)
{
	const MatcherHolder* matcher = query_->matcher();
	pointer ppRoot = pTile_ + 8 + indexType * 4;
	int32_t ptr = ppRoot.getInt();
	if (ptr == 0) return;
	if ((ptr & 1) == 0)
	{
		// method_ = matcher->method(indexType);
		searchRoot(ppRoot);
		return;
	}

	pointer p = ppRoot + (ptr ^ 1);
	for (;;)
	{
		int32_t last = p.getInt() & 1;
		int32_t keys = p.getInt(4);
		if (matcher->acceptIndex(indexType, keys))
		{
			searchRoot(p);
		}
		if (last != 0) break;
		p += 8;
	}
}


void TileQueryTask::searchRoot(pointer ppRoot)
{
	int32_t ptr = ppRoot.getInt();
	if (ptr)
	{
		pointer p = ppRoot + (ptr & 0xffff'fffc);
		if (ptr & 2)
		{
			searchLeaf(p);
		}
		else
		{
			searchBranch(p);
		}
	}
}

void TileQueryTask::searchBranch(pointer p)
{
	Box box = query_->bounds();
	for (;;)
	{
		int32_t ptr = p.getInt();
		int32_t last = ptr & 1;
		if (box.intersects(*reinterpret_cast<const Box*>((const uint8_t*)p + 4)))
		{
			pointer pChild = p + (ptr & 0xffff'fffc);
			if (ptr & 2)
			{
				searchLeaf(pChild);
			}
			else
			{
				searchBranch(pChild);
			}
		}
		if (last != 0) break;
		p += 20;
	}
}


void TileQueryTask::searchLeaf(pointer p)
{
	Box box = query_->bounds();
	FeatureTypes acceptedTypes = query_->types();
	const Matcher& matcher = query_->matcher()->mainMatcher();

	for (;;)
	{
		int32_t flags = p.getInt(16);
		int32_t multiTileFlags = flags & 
			(FeatureFlags::MULTITILE_NORTH | FeatureFlags::MULTITILE_WEST);
		for (;;)
		{
			int32_t dupeFlag = 0;
			if (multiTileFlags)
			{
				if (multiTileFlags == FeatureFlags::MULTITILE_WEST)
				{
					// If the feature has a second copy in the tile
					// to the west, and the query's bounding box
					// extends into that tile, we skip the feature

					if (tipAndFlags_ & FeatureFlags::MULTITILE_WEST) break;
				}
				else if (multiTileFlags == FeatureFlags::MULTITILE_NORTH)
				{
					// If the feature has a second copy in the tile
					// to the north, and the query's bounding box
					// extends into that tile, we skip the feature

					if (tipAndFlags_ & FeatureFlags::MULTITILE_NORTH) break;
				}
				else
				{
					// If both flags are set, this means we'll have
					// to add the feature to the deduplication set
					// TODO: if query does not extend beyond the
					// tile boundaries, we don't have to do this
					dupeFlag = Query::REQUIRES_DEDUP;
				}
			}
			if (!(p.getInt() > box.maxX() ||
				p.getInt(4) > box.maxY() ||
				p.getInt(8) < box.minX() ||
				p.getInt(12) < box.minY()))
			{
				// TODO: replace this branching code with arithmetic?
				// Useful? https://stackoverflow.com/a/62852710

				if (acceptedTypes.acceptFlags(flags))
				{
					pointer pFeature = p + 16;
					if (matcher.accept(pFeature))
					{
						const Filter* filter = query_->filter();
						if (filter == nullptr || filter->accept(query_->store(), 
							FeatureRef(pFeature), fastFilterHint_))
						{
							// LOG("Found %s/%llu", Feature::typeName(pFeature), Feature::id(pFeature));
							addResult((pFeature - pTile_) | dupeFlag);	// TODO
						}
					}
				}
			}
			break;
		}
		if (flags & 1) break;
		p += 32;
	}

}

/**
 * Add a relative pointer to the list of results.
 * If the current bucket is full, place a new bucket at the end
 * of the circular linked list of buckets.
 * - `results_` always points to the last bucket
 */
void TileQueryTask::addResult(uint32_t item)
{
	if (results_->isFull())
	{
		QueryResults* next = new QueryResults();
		QueryResults* last = (results_ == QueryResults::EMPTY) ? next : results_;
		next->count = 0;
		next->pTile = pTile_;
		next->next = last->next;
		last->next = next;
		results_ = next;
	}
	results_->items[results_->count++] = item;
}

