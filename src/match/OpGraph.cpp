// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#include "OpGraph.h"

OpGraph::OpGraph() :
	firstRegex_(nullptr),
	arena_(1024)		// TODO: size to multiple of OpNode
{
}


OpGraph::~OpGraph()
{
	RegexOperand* p = firstRegex_;
	while (p)
	{
		RegexOperand* next = p->next();
		delete p;
		p = next;
	}
}


RegexOperand* OpGraph::addRegex(const char* s, int len)
{
	RegexOperand* p = new RegexOperand(s, len, firstRegex_);
	firstRegex_ = p;
	return p;
}

const char* OPCODE_NAMES[] =
{
	"NOP",
	"EQ_CODE",
	"EQ_STR",
	"STARTS_WITH",
	"ENDS_WITH",
	"CONTAINS",
	"REGEX",
	"EQ_NUM",
	"LE",
	"LT",
	"GE",
	"GT",
	"GLOBAL_KEY",
	"FIRST_GLOBAL_KEY",
	"LOCAL_KEY",
	"FIRST_LOCAL_KEY",
	"HAS_LOCAL_KEYS",
	"LOAD_CODE",
	"LOAD_STRING",
	"LOAD_NUM",
	"CODE_TO_STR",
	"STR_TO_NUM",
	"FEATURE_TYPE",
	"GOTO",
	"RETURN"
};


uint8_t OPCODE_ARGS[] =
{
	0, // NOP
	2, // EQ_CODE
	2, // EQ_STR
	2, // STARTS_WITH
	2, // ENDS_WITH
	2, // CONTAINS
	2, // REGEX
	2, // EQ_NUM
	2, // LE
	2, // LT
	2, // GE
	2, // GT
	2, // GLOBAL_KEY
	2, // FIRST_GLOBAL_KEY
	2, // LOCAL_KEY
	2, // FIRST_LOCAL_KEY
	1, // HAS_LOCAL_KEYS
	1, // LOAD_CODE
	1, // LOAD_STRING
	1, // LOAD_NUM
	0, // CODE_TO_STR
	1, // STR_TO_NUM
	3, // FEATURE_TYPE (2-word operand)
	1, // GOTO
	0  // RETURN (argument is stored in flags)
};


OperandType OPCODE_OPERAND_TYPES[] =
{
	OperandType::NONE, // NOP
	OperandType::CODE, // EQ_CODE
	OperandType::STRING, // EQ_STR
	OperandType::STRING, // STARTS_WITH
	OperandType::STRING, // ENDS_WITH
	OperandType::STRING, // CONTAINS
	OperandType::REGEX, // REGEX
	OperandType::DOUBLE, // EQ_NUM
	OperandType::DOUBLE, // LE
	OperandType::DOUBLE, // LT
	OperandType::DOUBLE, // GE
	OperandType::DOUBLE, // GT
	OperandType::CODE, // GLOBAL_KEY
	OperandType::CODE, // FIRST_GLOBAL_KEY
	OperandType::STRING, // LOCAL_KEY
	OperandType::STRING, // FIRST_LOCAL_KEY
	OperandType::NONE, // HAS_LOCAL_KEYS
	OperandType::NONE, // LOAD_CODE
	OperandType::NONE, // LOAD_STRING
	OperandType::NONE, // LOAD_NUM
	OperandType::NONE, // CODE_TO_STR
	OperandType::NONE, // STR_TO_NUM
	OperandType::FEATURE_TYPES, // FEATURE_TYPE (2-word operand)
	OperandType::NONE, // GOTO
	OperandType::NONE  // RETURN (argument is stored in flags)
};




OpNode* OpGraph::createGoto(OpNode* target)
{
	OpNode* node = newOp(Opcode::GOTO);
	node->next[0] = target;
	node->next[1] = target;		// TODO: check !!!
	return node;
}


int OpNode::compareTo(const OpNode* other) const
{
	int comp = ((int)opcode) - other->opcode;
	if (comp != 0) return comp;
	switch (OPCODE_OPERAND_TYPES[opcode])
	{
		case OperandType::CODE:
			return (int)operand.code - other->operand.code;
		case OperandType::STRING:
		{
			std::string_view sv1(operand.string, operandLen);
			std::string_view sv2(other->operand.string, other->operandLen);
			return sv1.compare(sv2);
		}
		case OperandType::DOUBLE:
		{
			if (operand.number < other->operand.number)
			{
				return -1;
			}
			else if (operand.number > other->operand.number)
			{
				return 1;
			}
			return 0;
		}
		// TODO: feature type
		default:
			return -1;		// Regex are never compared for equality, always stack 
			// one after the other
	}
}

