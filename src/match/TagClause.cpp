// Copyright (c) 2023 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#include "TagClause.h"


TagClause::TagClause(int keyCode, int cat) :
	next(nullptr),
	keyOp(Opcode::GLOBAL_KEY),
	trueOp(Opcode::RETURN),
	category(cat),
	flags(0)
{
	keyOp.operand.code = keyCode;
	trueOp.operand.code = 1;
	keyOp.next[1] = &trueOp;
}

TagClause::TagClause(std::string_view key) :
	next(nullptr),
	keyOp(Opcode::LOCAL_KEY),
	trueOp(Opcode::RETURN),
	category(0),
	flags(0)
{
	keyOp.setStringOperand(key);
	trueOp.operand.code = 1;
	keyOp.next[1] = &trueOp;
}



const int TagClause::OPCODE_VALUE_TYPES[] =
{
	0, // NOP
	Flags::VALUE_GLOBAL_STRING, // EQ_CODE
	Flags::VALUE_LOCAL_STRING, // EQ_STR
	Flags::VALUE_ANY_STRING, // STARTS_WITH
	Flags::VALUE_ANY_STRING, // ENDS_WITH
	Flags::VALUE_ANY_STRING, // CONTAINS
	Flags::VALUE_ANY_STRING, // REGEX
	Flags::VALUE_ANY_NUMBER, // EQ_NUM
	Flags::VALUE_ANY_NUMBER, // LE
	Flags::VALUE_ANY_NUMBER, // LT
	Flags::VALUE_ANY_NUMBER, // GE
	Flags::VALUE_ANY_NUMBER, // GT
	// rest not needed, applies to value check opcodes only
};



OpNode** TagClause::insertValueOp(OpNode* node, bool asAnd)
{
	flags |= OPCODE_VALUE_TYPES[node->opcode];

	bool negated = keyOp.isNegated();
	OpNode* firstValueOp = keyOp.next[!negated];
	if (firstValueOp->opcode == Opcode::RETURN)
	{
		// There are no other value instructions
		node->next[0] = keyOp.next[0];
		node->next[1] = keyOp.next[1];
		keyOp.next[!negated] = node;
		return &node->next[asAnd];
	}

	OpNode** pNext = &keyOp.next[!keyOp.isNegated()];
	node->next[0] = firstValueOp->next[0];
	node->next[1] = firstValueOp->next[1];

	OpNode* current;
	for (;;)
	{
		current = *pNext;
		if (current->opcode == Opcode::RETURN) break;
		int comp = current->compareTo(node);
		if (comp > 0) break;
		if (comp == 0)
		{
			// TODO: negation check?
			return &current->next[asAnd];
		}
		pNext = &current->next[asAnd];
	}
	node->next[asAnd] = current;
	*pNext = node;
	assert(node->next[0] != node->next[1]);
	return &node->next[asAnd];
}


/**
 * Checks whether this clause matches multiple terms, e.g. [k=a,b]
 */
bool TagClause::isOrClause() const
{
	const OpNode* valOp = keyOp.next[1];
	if (valOp->opcode == Opcode::RETURN) return false;
	return valOp->next[0]->opcode != Opcode::RETURN;
}


/**
 * Merges two tag clauses together, e.g. [k>3][k<5] or [k][k!=a]
 */
void TagClause::absorb(TagClause* other)
{
	assert(keyOp.opcode == other->keyOp.opcode);
	if (keyOp.isNegated() && !other->keyOp.isNegated())
	{
		keyOp.setNegated(false);
		std::swap(keyOp.next[0], keyOp.next[1]);
	}
	flags |= other->flags;
	if ((flags & COMPLEX_BOOLEAN_CLAUSE) != 0 || isOrClause() ||
		other->isOrClause())
	{
		// If one or both of the clauses has an OR-chain of value ops
		// (e.g. [k!=a,b][k!=c,d]), chain the opcode strands without sorting
		// (This is very rare, but we have to support it per spec)
		// Note that we only link the true-target of the first op in an OR-chain.
		// The Validator will properly link all ops when it is creating the
		// type-specific chains

		flags |= COMPLEX_BOOLEAN_CLAUSE;
		OpNode* firstValOp = keyOp.next[1];
		OpNode* valOp = other->keyOp.next[1];
		keyOp.next[1] = valOp;
		for (;;)
		{
			OpNode* next = valOp->next[1];
			if (next->opcode == Opcode::RETURN) break;
			valOp = next;
		}
		valOp->next[1] = firstValOp;
		return;
	}

	// Otherwise, combine the two chains as AND-chains, sorting the 
	// value ops

	bool otherNegated = other->keyOp.isNegated();
	OpNode* valOp = other->keyOp.next[!otherNegated];
	OpNode** pNext = &keyOp.next[!keyOp.isNegated()];
	while (valOp->opcode != Opcode::RETURN)
	{
		pNext = insertValueOp(valOp, true);
		valOp = valOp->next[1];
	}
}


