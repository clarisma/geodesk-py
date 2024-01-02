// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include "OpGraph.h"
#include "Selector.h"
#include "TagClause.h"

class MatcherValidator
{
public:
	MatcherValidator(OpGraph& graph);

	OpNode* validate(Selector* firstSel);

	uint32_t resourceSize() const { return resourceSize_; }
	uint32_t regexCount() const { return regexCount_; }
	uint32_t maxInstructionSize() const 
	{ 
		return (totalInstructionWords_ + maxExtraGotos_ * 2) * 2; 
	}
	FeatureTypes featureTypes() { return featureTypes_; }

private:
	void validateOp(OpNode* node);

	OpNode* validateAllSelectors(Selector* first);
	OpNode* validateSelector(Selector* sel);
	void insertLoadOps(TagClause* clause);
	OpNode* createValueOps(const OpNode* keyOp, uint32_t acceptedValues);
	OpNode* cloneValueOps(const OpNode* valOps, uint32_t acceptedValues, OpNode* falseOp);

	OpGraph& graph_;
	uint32_t totalInstructionWords_;
	uint32_t maxExtraGotos_;
	uint32_t regexCount_;
	uint32_t resourceSize_;
	FeatureTypes featureTypes_;
	uint32_t featureTypeOpCount_;
};
