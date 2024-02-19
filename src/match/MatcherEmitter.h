// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once
#include "OpGraph.h"
#include <common/alloc/ArenaPool.h>
#include <common/alloc/ArenaStack.h>

struct StringResource
{
	uint16_t len;
	char data[1];		// variable-length
};


// TODO: respect the order of resources!
// - pointers to other matchers come first
// - then Regex patterns
// - then doubles and strings

class MatcherResourceAllocator
{
public:
	MatcherResourceAllocator(uint8_t* pResources) :
		pResourcesStart_(pResources),
		pResourcesEnd_(pResources)
	{
	}

	double* allocDouble()
	{
		return (double*)alloc(sizeof(double));
	}

	std::regex* allocRegex(RegexOperand* pRegexOperand)		
	{
		// TODO: must use a special area at front of resources!
		std::regex* pRegex = reinterpret_cast<std::regex*>(alloc(sizeof(std::regex)));
		new (pRegex) std::regex(std::move(pRegexOperand->regex()));
		pRegexOperand->setRegexResource(pRegex);
		return pRegex;
	}

	StringResource* allocString(uint16_t len)
	{
		return (StringResource*)alloc((size_t)len + 2);
	}


private:
	uint8_t* alloc(size_t size)
	{
		size += (8 - (size & 7)) & 7;
		uint8_t* p = pResourcesEnd_;
		pResourcesEnd_ += size;
		return p;
	}

	uint8_t* pResourcesStart_;
	uint8_t* pResourcesEnd_;
};

// TODO: Bug: deferred_ & jumps_ both use prev to create a stack
//  Need a way to remove an op from deferred_ when it is being placed

class MatcherEmitter
{
public:
	MatcherEmitter(OpGraph& graph, OpNode* root, uint8_t* pResources, uint16_t* pCode);
	void emit();
	void fixJumps();

private:
	inline void defer(OpNode* node)
	{
		if (node->flags & OpFlags::DEFERRED) return;
		node->flags |= OpFlags::DEFERRED;
		assert(_CrtCheckMemory());
		deferred_.push(node);
		assert(_CrtCheckMemory());
	}

	inline OpNode* takeDeferred()
	{
		while(!deferred_.isEmpty())
		{
			OpNode* node = deferred_.pop();
			if (node->address == 0) return node;
		}
		return nullptr;
	}

	inline void addJump(OpNode* node)
	{
		jumps_.push(node);
		assert(_CrtCheckMemory());
	}

	void putResourceOffset(uint16_t* pOperand, const void* pResource)
	{
		ptrdiff_t ofs = reinterpret_cast<uint8_t*>(pOperand) -
			reinterpret_cast<const uint8_t*>(pResource);
		assert(ofs > 0);
		assert(ofs < 0xffff);
		*pOperand = static_cast<uint16_t>(ofs);
	}

	void createRegexResources();

	static const int STACK_CHUNK_SIZE = 32;

	using OpNodeStack = ArenaStack<OpNode*, STACK_CHUNK_SIZE>;

	OpGraph& graph_;
	OpNode* root_;
	OpNodeStack::Pool pool_;
	OpNodeStack deferred_;
	OpNodeStack jumps_;
	MatcherResourceAllocator resources_;
	uint16_t* pCode_;
};
