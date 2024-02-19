// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#include "MatcherValidator.h"

MatcherValidator::MatcherValidator(OpGraph& graph) :
	graph_(graph),
	totalInstructionWords_(0),
	maxExtraGotos_(0),
	regexCount_(0),
	resourceSize_(0),
	featureTypes_(0),
	featureTypeOpCount_(0)
{
}


OpNode* MatcherValidator::validate(Selector* firstSel)
{
	assert(_CrtCheckMemory());
	RegexOperand* pRegex = graph_.firstRegex();
	while (pRegex)
	{
		regexCount_++;
		resourceSize_ += (sizeof(std::regex) + 7) & 0xffff'fff8;
		pRegex = pRegex->next();
	}

	OpNode* root = validateAllSelectors(firstSel);
	validateOp(root);
	assert(_CrtCheckMemory());
	return root;
}



// TODO: may need explciit stack because complex queries may have 
// a call depth that is too deep
void MatcherValidator::validateOp(OpNode* node)
{
	node->flags |= OpFlags::VALIDATED;
	int op = node->opcode;
	totalInstructionWords_ += OPCODE_ARGS[op] + 1;
	switch (OPCODE_OPERAND_TYPES[op])
	{
	case OperandType::DOUBLE:
		static_assert(sizeof(double) == 8, "Expected 8-byte double");
		resourceSize_ += sizeof(double);
		break;
	case OperandType::STRING:
		resourceSize_ += (node->operandLen + 2 + 7) & 0xffff'fff8;
		break;
	}
	
	bool multipleCallersToFalse = false;
	OpNode* falseOp = node->next[0];
	OpNode* trueOp = node->next[1];
	if (falseOp)
	{
		if(!falseOp->isValidated()) validateOp(falseOp);
		falseOp->callerCount++;
		multipleCallersToFalse = falseOp->callerCount > 1;
	}
	if (trueOp)
	{
		if (!trueOp->isValidated()) validateOp(trueOp);
		trueOp->callerCount++;
		if (multipleCallersToFalse && trueOp->callerCount > 1)
		{
			maxExtraGotos_++;
		}
	}
}


/**
 * Inserts and links up LOAD_CODE, LOAD_STR or LOAD_NUM as needed based
 * on the operand types of the value ops (summarized in flags). Also
 * inserts CODE_TO_STR and STR_TO_NUM, if needed.
 */
void MatcherValidator::insertLoadOps(TagClause* clause)
{
	OpNode* keyOp = &clause->keyOp;
	bool negated = keyOp->isNegated();
	
	OpNode* firstValueOp = keyOp->next[!negated];
	// For k = a, value ops are evaluated if key-op is true
	// For k != a, value ops are evaluated if key-op is false
	OpNode* wrongTypeOp = keyOp->next[negated];
	// For k = a, we jump to false-target if tag has wrong type
	// For k != a, we jump to true-target if tag has wrong type

	OpNode* loadOp = wrongTypeOp;
	uint32_t clauseFlags = clause->flags;
	if (clauseFlags == TagClause::VALUE_GLOBAL_STRING)		// only code (== is ok)
	{
		// Most common case: all value-ops are EQ_CODE

		// for unary clauses ([k] or [!k]), we jump to the success target
		// if load of global-code fails (because it cannot be "no")
		// TODO: make this code cleaner

		bool isUnaryOp = negated != firstValueOp->isNegated();
		wrongTypeOp = isUnaryOp ? firstValueOp->next[!negated] : keyOp->next[negated];
		loadOp = graph_.newOp(Opcode::LOAD_CODE, wrongTypeOp, firstValueOp);
	}
	else if (clauseFlags == TagClause::VALUE_LOCAL_STRING)  // only string (== is ok)
	{
		// all value-ops are EQ_STR
		loadOp = graph_.newOp(Opcode::LOAD_STRING, wrongTypeOp, firstValueOp);
	}
	else if (clauseFlags == TagClause::VALUE_ANY_NUMBER)	// only numeric (== is ok)
	{
		// For numeric ops, create a chain that first checks for
		// number, then wide string (converting string to num),
		// and finally code (converting first to string, then to num)

		OpNode* strToNumOp = graph_.newOp(Opcode::STR_TO_NUM, wrongTypeOp, firstValueOp);
		OpNode* codeToStrOp = graph_.newOp(Opcode::CODE_TO_STR, strToNumOp, nullptr);
		OpNode* loadCodeOp = graph_.newOp(Opcode::LOAD_CODE, wrongTypeOp, codeToStrOp);
		OpNode* loadStringOp = graph_.newOp(Opcode::LOAD_STRING, loadCodeOp, strToNumOp);
		loadOp = graph_.newOp(Opcode::LOAD_NUM, loadStringOp, firstValueOp);
	}
	else
	{
		// Create a path for each type
		if (clauseFlags & TagClause::VALUE_ANY_NUMBER)
		{
			OpNode* op = createValueOps(keyOp, TagClause::VALUE_ANY_NUMBER);
			if (op)
			{
				loadOp = graph_.newOp(Opcode::LOAD_NUM, loadOp, op);
			}
		}
		if (clauseFlags & (TagClause::VALUE_LOCAL_STRING |
			TagClause::VALUE_ANY_STRING | TagClause::VALUE_ANY_NUMBER))
		{
			OpNode* op = createValueOps(keyOp, (TagClause::VALUE_LOCAL_STRING |
				TagClause::VALUE_ANY_STRING | TagClause::VALUE_ANY_NUMBER));
			if (op)
			{
				if (clauseFlags & TagClause::VALUE_ANY_NUMBER)
				{
					op = graph_.newOp(Opcode::STR_TO_NUM, op, op);
					// If string is not a number, the double value will be NaN;
					// for simplicity, we can continue to value checks in both
					// false and true case
				}
				loadOp = graph_.newOp(Opcode::LOAD_STRING, loadOp, op);
			}
		}
		if (clauseFlags & (TagClause::VALUE_GLOBAL_STRING |
			TagClause::VALUE_ANY_STRING | TagClause::VALUE_ANY_NUMBER))
		{
			OpNode* op = createValueOps(keyOp, (TagClause::VALUE_GLOBAL_STRING |
				TagClause::VALUE_ANY_STRING | TagClause::VALUE_ANY_NUMBER));
			if (op)
			{
				if (clauseFlags & TagClause::VALUE_ANY_NUMBER)
				{
					op = graph_.newOp(Opcode::STR_TO_NUM, op, op);
					// If string is not a number, the double value will be NaN;
					// for simplicity, we can continue to value checks in both
					// false and true case
				}
				if (clauseFlags & (TagClause::VALUE_ANY_STRING | TagClause::VALUE_ANY_NUMBER))
				{
					op = graph_.newOp(Opcode::CODE_TO_STR, op, nullptr);
				}
				loadOp = graph_.newOp(Opcode::LOAD_CODE, loadOp, op);
			}
		}
	}
	keyOp->next[!negated] = loadOp;
}

/**
 * Creates a copy of an OR-chain of value ops, consisiting only of ops that
 * apply to acceptedValues (TagClause::Flags)
 * Returns null if no ops in the chain apply to the given values.
 */
OpNode* MatcherValidator::cloneValueOps(
	const OpNode* valOps, uint32_t acceptedValues, 
	OpNode* falseOp)
{
	const OpNode* valOp = valOps;
	OpNode* firstClonedOp = nullptr;
	OpNode** pNext = &firstClonedOp;
	while (valOp->opcode != Opcode::RETURN)
	{
		if (TagClause::OPCODE_VALUE_TYPES[valOp->opcode] & acceptedValues)
		{
			OpNode* clonedOp = graph_.copyOp(valOp);
			clonedOp->next[0] = falseOp;
			*pNext = clonedOp;
			pNext = &clonedOp->next[0];
		}
		valOp = valOp->next[0];
	}
	return firstClonedOp;
}

OpNode* MatcherValidator::createValueOps(const OpNode* keyOp, uint32_t acceptedValues)
{
	OpNode* firstClonedOp = nullptr;
	OpNode** pNext = &firstClonedOp;
	OpNode* prev = nullptr;
	bool negated = keyOp->isNegated();
	const OpNode* valOp = keyOp->next[!negated];
	OpNode* falseOp = keyOp->next[negated];

	while (valOp->opcode != Opcode::RETURN)
	{
		OpNode* clonedOp = cloneValueOps(valOp, acceptedValues, falseOp);
		if (!clonedOp) return nullptr;
		*pNext = clonedOp;
		pNext = &clonedOp->next[1];

		if (prev)
		{
			while (prev->opcode != Opcode::RETURN)
			{
				prev->next[1] = clonedOp;
				prev = prev->next[0];
			}
		}
		prev = clonedOp;
		valOp = valOp->next[1];
	}
	return firstClonedOp;
}


/**
 * Creates load ops for the accepted values of each tag clause, then links
 * together the tag clauses by copying the first key-op of each clause into
 * the true-op of the preceding clause. If a selector has local-key ops,
 * we'll insert a HAS_LOCAL_KEYS op. We also turn the first key-op of each 
 * kind into a FIRST_xxx_KEY op.
 */
OpNode* MatcherValidator::validateSelector(Selector* sel)
{
	featureTypes_ |= sel->acceptedTypes;
	TagClause* clause = sel->firstClause;
	TagClause* lastClause = nullptr;
	TagClause* lastGlobalKeyClause = nullptr;
	bool seenGlobalKeyOp = false;
	bool seenLocalKeyOp = false;
	bool allLocalKeyOpsNegated = true;
	while (clause)
	{
		if (clause->keyOp.opcode == Opcode::GLOBAL_KEY)
		{
			if (!seenGlobalKeyOp)
			{
				clause->keyOp.opcode = Opcode::FIRST_GLOBAL_KEY;
				seenGlobalKeyOp = true;
			}
			lastGlobalKeyClause = clause;
		}
		else
		{
			assert(clause->keyOp.opcode == Opcode::LOCAL_KEY);
			if (!seenLocalKeyOp)
			{
				clause->keyOp.opcode = Opcode::FIRST_LOCAL_KEY;
				seenLocalKeyOp = true;
			}
			if (!clause->keyOp.isNegated()) allLocalKeyOpsNegated = false;
		}

		insertLoadOps(clause);
		if (lastClause)
		{
			lastClause->trueOp = clause->keyOp;
			// Rather than re-writing the links from multiple val-ops,
			// we simply copy the key-op of this clause into the true-op 
			// of the preceding clause (which was "RETURN true")
		}
		lastClause = clause;
		clause = clause->next;
	}

	OpNode* op = &sel->firstClause->keyOp;
	if (seenLocalKeyOp)
	{
		if (!allLocalKeyOpsNegated)
		{
			// The selector fails if no local keys are present
			op = graph_.newOp(Opcode::HAS_LOCAL_KEYS, &sel->falseOp, op);
		}
		else
		{
			if (!seenGlobalKeyOp)
			{
				// The selector has only local-key clauses, and they are all
				// negative (they succeed if local-key tag is not present)
				// Hence the selector always succeeds if no local keys are present
				op = graph_.newOp(Opcode::HAS_LOCAL_KEYS, &lastClause->trueOp, op);
			}
			else
			{
				// We place the local-key check before the first local-key op
				assert(lastGlobalKeyClause);
				assert(lastGlobalKeyClause->next);
				lastGlobalKeyClause->trueOp.opcode = Opcode::HAS_LOCAL_KEYS;
				lastGlobalKeyClause->trueOp.next[1] = &lastGlobalKeyClause->next->keyOp;
			}
		}
	}
	return op;
}


/**
 * Validates each individual selector, then copies the first op of each selector
 * into the false-op of the preceding selector. If the first op of each selector
 * is a HAS_LOCAL_KEYS op, and they all are the same (all of them fail if no
 * local keys present, or all of them succeed if no local keys present), we
 * remove the redundant HAS_LOCAL_KEYS ops of the subsequent selectors.
 */
OpNode* MatcherValidator::validateAllSelectors(Selector* firstSel)
{
	OpNode* firstOp = validateSelector(firstSel);
	if (firstSel->next == nullptr) return firstOp;	// only one selector

	Selector* sel = firstSel;
	bool canMergeLocalKeyChecks = (firstOp->opcode == Opcode::HAS_LOCAL_KEYS);
	for(;;)
	{
		Selector* prevSel = sel;
		sel = sel->next;
		if (!sel) break;
		OpNode* selOp = validateSelector(sel);
		prevSel->falseOp = *selOp;
			// Copy first op of selector into the false-op of the 
			// previous selector
		if (selOp->opcode != Opcode::HAS_LOCAL_KEYS ||
			selOp->next[0]->operand.code != firstOp->next[0]->operand.code)
		{
			canMergeLocalKeyChecks = false;
		}
	}
	if (canMergeLocalKeyChecks)
	{
		// The HAS_LOCAL_KEYS op is the same for all selectors, so we can
		// omit it for the subsequent selectors

		Selector* sel = firstSel;
		for (;;)
		{
			Selector* nextSel = sel->next;
			if (!nextSel) break;
			sel->falseOp = nextSel->firstClause->keyOp;
				// Copy the first key-op of the next selector into
				// the true-op of this selector, thereby replacing the
				// redundant HAS_LOCAL_KEYS check
			sel = nextSel;
		}
		if (firstOp->next[0]->opcode != Opcode::RETURN)
		{
			// ensure that the common HAS_LOCAL_KEYS op branches to
			// a return statement instead of going to the second selector
			// (TODO: This feels hacky)
			firstOp->next[0] = &sel->falseOp;
		}
	}
	return firstOp;
}
