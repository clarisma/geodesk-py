// Copyright (c) 2023 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#include "MatcherEmitter.h"
#include <string>
#include <common/compile/unreachable.h>

MatcherEmitter::MatcherEmitter(OpGraph& graph, OpNode* root, uint8_t* pResources, uint16_t* pCode) :
	graph_(graph),
	root_(root),
	resources_(pResources),
	pCode_(pCode),
	pool_(graph.arena()),
	deferred_(pool_),
	jumps_(pool_)
{
}


// TODO: must guarantee address is zero before emitting, is unioned with copy
// field which will set it nonzero

void MatcherEmitter::emit()
{
	OpNode* node = root_;
	uint16_t* pOpcode;
	uint16_t* p = pCode_;

	for (;;)
	{
		node->flags |= OpFlags::EMITTED;
		pOpcode = p;
		p++;
		assert(node->address == 0);
		node->address = (pOpcode - pCode_) * 2;	// in bytes
		int opcode = node->opcode;
		switch (opcode)
		{
			// non-branching ops
		case Opcode::NOP:
		case Opcode::CODE_TO_STR:
		{
			*pOpcode = opcode;
			OpNode* next = node->next[0];
			if (next->address)		// target has already been placed -> insert a goto
			{
				next = node->next[0] = graph_.createGoto(next);
			}
			node = next;
		}
		continue;

			// branch based on <code> operand
		case Opcode::EQ_CODE:
		case Opcode::GLOBAL_KEY:
		case Opcode::FIRST_GLOBAL_KEY:
			*p++ = node->operand.code;
			break;

			// branch based on <string> operand
		case Opcode::EQ_STR:
		case Opcode::STARTS_WITH:
		case Opcode::ENDS_WITH:
		case Opcode::CONTAINS:
		case Opcode::LOCAL_KEY:
		case Opcode::FIRST_LOCAL_KEY:
		{
			uint16_t len = node->operandLen;
			StringResource* str = resources_.allocString(len);
			str->len = len;
			std::memcpy(str->data, node->operand.string, len);
			putResourceOffset(p++, str);
		}
		break;

			// branch based on <regex> operand
		case Opcode::REGEX:
		{
			std::regex* pRegex = resources_.allocRegex();
			new (pRegex) std::regex(std::move(node->operand.regex->regex()));
			putResourceOffset(p++, pRegex);
		}
		break;

			// branch based on <number> operand
		case Opcode::EQ_NUM:
		case Opcode::LE:
		case Opcode::LT:
		case Opcode::GE:
		case Opcode::GT:
		{
			double* pDouble = resources_.allocDouble();
			*pDouble = node->operand.number;
			putResourceOffset(p++, pDouble);
		}
		break;

			// branch without operand
		case Opcode::HAS_LOCAL_KEYS:
		case Opcode::LOAD_CODE:
		case Opcode::LOAD_STRING:
		case Opcode::LOAD_NUM:
		case Opcode::STR_TO_NUM:
			// nothing else to do
			break;

		case Opcode::FEATURE_TYPE:
		{
			uint32_t types = node->operand.featureTypes;
			std::memcpy(p, &types, sizeof(uint32_t));
			p += 2;
		}
		break;

		case Opcode::GOTO:
			*pOpcode = Opcode::GOTO;
			// assert(node->next[1] == nullptr);
			p++;		
			addJump(node);
			node = takeDeferred();
			if (!node) return;
			continue; 

		case Opcode::RETURN:
			assert(node->next[0] == nullptr);
			assert(node->next[1] == nullptr);
			*pOpcode = opcode | (node->operand.code << 8);
			node = takeDeferred();
			if (!node) return;
			continue;

		default:
			UNREACHABLE_CASE
		}

		// TODO: what if true and false are same target (e.g. str2num
		//  if we don't care about outcome?)

		OpNode* ifFalse = node->next[0];
		OpNode* ifTrue = node->next[1];

		assert(ifFalse);
		assert(ifTrue);

		assert(ifTrue != ifFalse || opcode==Opcode::STR_TO_NUM);   // str_to_num can go to same target (because invalid strign becomes NaN)

		*pOpcode = opcode | (node->isNegated() ? 256 : 0);
		p++;				// slot for the jump address
		
		if (ifTrue->address)
		{
			// True branch has been placed already;

			if (ifFalse->address)
			{
				// both branches have been placed
				ifFalse = node->next[0] = graph_.createGoto(ifFalse);
			}
		}
		else
		{
			// True branch hasn't been placed

			if (ifFalse->address != 0 || ifFalse->callerCount > ifTrue->callerCount)
			{
				// False branch has been placed or more instructions
				// branch to it --> make this instruction branch to false

				node->inverse();	// TODO: should inverseNegate flip t/f as well?
				*pOpcode ^= 256;		// inverse the placed opcode as well
				std::swap(ifTrue, ifFalse);
			}
		}

		defer(ifTrue);
		addJump(node);
		node = ifFalse;
	}
}


void MatcherEmitter::fixJumps()
{
	while (!jumps_.isEmpty())
	{
		OpNode* node = jumps_.pop();
		// Calculate the offset (in bytes) where the relative jump address is stored
		// (OpNode::address is always in bytes, not words)
		int branchArgOffsetInBytes = node->address + OPCODE_ARGS[node->opcode] * 2;
		assert(branchArgOffsetInBytes % 2 == 0);
		int targetAddress = node->next[1]->address;
		assert(targetAddress % 2 == 0);
		int relativeJump = targetAddress - branchArgOffsetInBytes;
		assert(relativeJump != 0);
		assert(static_cast<int16_t>(relativeJump) == relativeJump);
		uint16_t* p = pCode_ + branchArgOffsetInBytes / 2;	// pointer uses words
		*p = static_cast<uint16_t>(relativeJump);
	}
}
