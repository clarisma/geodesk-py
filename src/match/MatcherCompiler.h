// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

// #define ASMJIT_STATIC 
// #include <asmjit/asmjit.h>
#include <cstdint>

class FeatureStore;
class MatcherHolder;
class OpGraph;
struct Selector;

class MatcherCompiler
{
public:
	explicit MatcherCompiler(FeatureStore* store) :
		store_(store)
	{
		// TODO: fix this dependency, store not initialized yet
	}

	const MatcherHolder* getMatcher(const char* query);

private:
	const MatcherHolder* compileMatcher(OpGraph& graph, Selector* firstSel, uint32_t indexBits);

	FeatureStore* store_;
	// asmjit::JitRuntime runtime_;
};


