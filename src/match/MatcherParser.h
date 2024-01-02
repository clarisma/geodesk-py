// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include "OpGraph.h"
#include "Selector.h"
#include "TagClause.h"
#include "feature/FeatureStore.h"
#include <common/util/Parser.h>

class MatcherParser : public Parser
{
public:
	MatcherParser(FeatureStore* store, const char* pInput);

	Selector* parse();
	OpGraph& graph() { return graph_; }
	uint32_t indexBits() const { return indexBits_; }
	int codeNo() const { return codeNo_; }

private:
	int getStringCode(std::string_view s)
	{
		return store_->strings().getCode(s.data(), s.length());
	}
	Selector* expectSelector();
	TagClause* expectTagClause();
	TagClause* expectKey();
	std::string_view acceptEscapedString();
	RegexOperand* expectRegex();
	OpNode* acceptStringOperand();
	FeatureTypes matchTypes();

	static const CharSchema VALID_FIRST_CHAR;
	static const CharSchema VALID_NEXT_CHAR;

	FeatureStore* store_;
	OpGraph graph_;
	Selector* currentSel_;
	uint32_t indexBits_;
		// TODO: It would be better to break out the index bits based on feature type,
		// which would allow polyform queries to use indexes more efficiently
		// (e.g. "na[amenity=fire_station), n[emergency=fire_hydrant]" could
		// avoid selecting buckets with "emergency" keys but no "amenity" keys
		// when querying areas), but these types of queries are rare and this
		// design is simpler
		
	// OpNode* selectorSuccess_;
	int codeNo_;

};

