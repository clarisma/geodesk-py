// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once
#include <regex>
#include <string_view>
#include <common/alloc/Arena.h>
#include "feature/types.h"

enum OpcodeTraits
{
	OPCODE_NOARG = 0,
	OPCODE_ONEARG = 1,
	OPCODE_TWOARG = 2,
	OPCODE_THREEARG = 3,
	OPCODE_EXACT = 8,
};

enum Opcode
{
	// Keep the order of these opcodes
	NOP,
	EQ_CODE,			// 1 TODO: need EQ_EXACT_NN, EQ_EXACT_WN
	EQ_STR,				// 2
	STARTS_WITH,        // 3
	ENDS_WITH,          // 4
	CONTAINS,           // 5
	REGEX,              // 6
	EQ_NUM,				// 7
	LE,					// 8
	LT,					// 9
	GE,					// 10
	GT,					// 11 If adding more value opcodes, change OpNode::isValueOp()
						// From here on, order is not relevant
	GLOBAL_KEY,			// 12
	FIRST_GLOBAL_KEY,	// 13
	LOCAL_KEY,
	FIRST_LOCAL_KEY,
	HAS_LOCAL_KEYS,
	LOAD_CODE,
	LOAD_STRING,
	LOAD_NUM,
	CODE_TO_STR,
	STR_TO_NUM,
	FEATURE_TYPE,		// 4-word instruction (opcode, int32, jump)
	GOTO,
	RETURN,
};

enum OpFlags
{
	/* Turns operation into a logical NOT */
	NEGATE = 1 << 0,	

	VALIDATED = 1 << 11,
	/*
	 *  
	 */
	EMITTED = 1 << 12,
	/**
	 * The operation has been referenced by another, but has not been placed.
	 */
	DEFERRED = 1 << 13,
};


enum class OperandType : uint8_t
{
	NONE,
	CODE,
	STRING,
	DOUBLE,
	REGEX,
	FEATURE_TYPES
};

/*
enum class TagValueType : uint8_t
{
	NONE = 0,
	GLOBAL_STRING = 1,
	LOCAL_STRING = 2,
	NARROW_NUMBER = 3,
	WIDE_NUMBER = 4,
	ANY_STRING = 5,
	ANY_NUMBER = 6
};
*/

class RegexOperand
{
public:
	RegexOperand(const char* s, int len, RegexOperand* next)
		: next_(next), regexResource_(nullptr), regex_(std::string(s, len)) {}
		// Must init next_ first to we have a valid chain in case
		// regex constructor fails
	
	std::regex& regex()  { return regex_; }
	RegexOperand* next() { return next_; }
	const std::regex* regexResource() const { return regexResource_; }
	void setRegexResource(const std::regex* pRegex) { regexResource_ = pRegex; }

private:
	/**
	 * The next regex operand in the linked list.
	 */
	RegexOperand* next_;

	/**
	 * Pointer to the regex in the MatcherHolder. This is initially null
	 * and will be assigned an adress by the MatcherEmitter.
	 */
	const std::regex* regexResource_;

	/**
	 * The compiled regex. Once parsing is successful, this regex will be
	 * transferred to regexResource_ (using move cosntruction) during 
	 * opcode generation. 
	 */
	std::regex regex_;
};

struct Operand
{
	union
	{
		uint16_t	  code;
		const char*   string;
		double        number;
		RegexOperand* regex;
		uint32_t      featureTypes;
	};
};

struct OpNode
{
	uint8_t  opcode;
	uint16_t operandLen;
	uint32_t flags;
	uint32_t address;
	uint32_t callerCount;
	Operand operand;
	OpNode* next[2];

	OpNode(int code)
	{
		memset(this, 0, sizeof(OpNode));
		opcode = static_cast<uint8_t>(code);
	}

	bool isValueOp() const { return opcode <= Opcode::GT; }
	bool isReturnFalseOp() const 
	{ 
		return opcode == Opcode::RETURN && operand.code==0; 
	}
	OpNode* nextIf(bool t) const { return next[t]; }
	void setNegated(bool t) { flags = (flags & ~OpFlags::NEGATE) | (t ? OpFlags::NEGATE : 0); }
	void setStringOperand(std::string_view sv)
	{
		operand.string = sv.data();
		operandLen = static_cast<uint16_t>(sv.length()); // TODO: enforce max str length
	}

	bool isNegated() const { return flags & OpFlags::NEGATE; }
	void inverse() 
	{ 
		flags ^= OpFlags::NEGATE; 
		std::swap(next[0], next[1]);
	}

	bool isValidated() const { return flags & OpFlags::VALIDATED; }

	int compareTo(const OpNode* other) const;
};

extern const char* OPCODE_NAMES[];
extern uint8_t OPCODE_ARGS[];
extern OperandType OPCODE_OPERAND_TYPES[];
// extern TagValueType OPCODE_TAG_VALUE_TYPES[];

class OpGraph
{
public:
	OpGraph();
	~OpGraph();

	Arena& arena() { return arena_; }

	RegexOperand* addRegex(const char* s, int len);
	RegexOperand* firstRegex() const { return firstRegex_;  }

	OpNode* createGoto(OpNode* target);

	OpNode* newOp(int op, std::string_view sv)
	{
		assert(OPCODE_OPERAND_TYPES[op] == OperandType::STRING);
		OpNode* node = newOp(op);
		node->setStringOperand(sv);
		return node;
	}

	OpNode* newOp(int op, int code)
	{
		assert(OPCODE_OPERAND_TYPES[op] == OperandType::CODE || op==Opcode::RETURN);
			// RETURN is a special case: in the OpNode, it stores its operand
			// as operand.code, but in the encoding its value becomes part of
			// the instruction word
		OpNode* node = newOp(op);
		node->operand.code = code;
		return node;
	}

	OpNode* newOp(int op, double num)
	{
		assert(OPCODE_OPERAND_TYPES[op] == OperandType::DOUBLE);
		OpNode* node = newOp(op);
		node->operand.number = num;
		return node;
	}

	OpNode* newOp(int op, RegexOperand* regex)
	{
		assert(OPCODE_OPERAND_TYPES[op] == OperandType::REGEX);
		OpNode* node = newOp(op);
		node->operand.regex = regex;
		return node;
	}

	OpNode* newOp(int op, FeatureTypes types)
	{
		assert(OPCODE_OPERAND_TYPES[op] == OperandType::FEATURE_TYPES);
		OpNode* node = newOp(op);
		node->operand.featureTypes = types;
		return node;
	}

	OpNode* newOp(int op)
	{
		OpNode* node = arena_.alloc<OpNode>();
		new(node)OpNode(op);
		return node;
	}

	OpNode* newOp(int op, OpNode* falseOp, OpNode* trueOp)
	{
		OpNode* node = arena_.alloc<OpNode>();
		new(node)OpNode(op);
		node->next[0] = falseOp;
		node->next[1] = trueOp;
		return node;
	}

	OpNode* copyOp(const OpNode* op)
	{
		OpNode* node = arena_.alloc<OpNode>();
		*node = *op;
		return node;
	}

private:
	Arena arena_;
	RegexOperand* firstRegex_;
};

