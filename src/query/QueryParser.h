// Copyright (c) 2023 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include <vector>
#include "util/Parser.h"
#include "feature/FeatureStore.h"

enum OpCode
{
	/**
	 * Branches if the feature's type matches the type pattern (stored as 
	 * 32-bit operand).
	 */
	FEATURE_TYPE,

	/**
	 * Checks if the current global tag matches the given global key (code).
	 * If so, advances tag pointer and jumps. 
	 * - Prior to this instruction, the tag pointer must point to a valid
	 *   global key, or the "end-of-table" flag must be set
	 * - After the instruction, the tag pointer points to the next global tag;
	 *   if there are no more global-key tags, the "end-of-table" flag is set
	 */
	GLOBAL_KEY,

	/**
	 * Sets the tag pointer to the start of global tags, then performs
	 * GLOBAL_KEY operation.
	 */
	FIRST_GLOBAL_KEY,

	/**
	 * Checks if the current local tag matches the given local key (string).
	 * If so, advances tag pointer and jumps.
	 * - Prior to this instruction, the tag pointer must point to a valid
	 *   local key, or the "end-of-table" flag must be set
	 * - After this instruction, the tag pointer points to the next local tag;
	 *   if there are no more local-key tags, the "end-of-table" flag is set
	 */
	LOCAL_KEY,
	
	/**
	 * Sets the tag pointer to the start of local tags, then performs
	 * LOCAL_KEY operation. 
	 * - The tag table must have local keys
	 */
	FIRST_LOCAL_KEY,

	/**
	 * Checks whether the tag-table has at least one local-key tag.
	 * If so, branches.
	 */
	HAS_LOCAL_KEYS,

	/**
	 * Checks whether the current tag has a global-string value.
	 * If so, loads the code and branches.
	 */
	LOAD_CODE,
	
	/**
	 * Checks whether the current tag has a local-string value.
	 * If so, loads the string and branches.
	 */
	LOAD_STR,
	
	/**
	 * Checks whether the current tag has a narrow or wide numeric value.
	 * If so, loads the number (as double value) and branches.
	 */
	LOAD_NUM,

	/**
	 * Loads the string value of a global-string code. 
	 * - code has to be loaded via LOAD_CODE
	 * - This instruction does not branch
	 */
	CODE_TO_STR,

	/**
	 * Checks whether a string value contains a number. If so, converts the
	 * string to a double value and branches.
	 */
	STR_TO_NUM,

	/**
	 * Branches if the global-key code equals the given code.
	 * - A global-string code must have been loaded via LOAD_CODE
	 */
	EQ_CODE,
	
	/**
	 * Branches if the string value equals the given string.
	 * - A string value must have been loaded via LOAD_STR, or converted from
	 *   a global-string code via CODE_TO_STR.
	 */
	EQ_STR,

	/**
	 * Branches if the double value equals the given double.
	 * - A double value must have been loaded via LOAD_NUM, or successfully 
	 *   converted from a string value via STR_TO_NUM.
	 */
	EQ_NUM,

	/**
	 * Branches if the double value is less than the given double.
	 */ 
	LT,
	LE,
	GT,
	GE,

	/**
	 * Branches if the string value starts with the given string (case sensitive).
	 */
	STARTS_WTH,

	/**
	 * Branches if the string value ends with the given string (case sensitive).
	 */
	ENDS_WITH,

	/**
	 * Branches if the string value contains the given string (case sensitive).
	 */
	CONTAINS,

	/**
	 * Branches if the string value matches the given regular expression.
	 */
	REGEX,

	/**
	 * The matcher succeeds.
	 */
	RET_TRUE,
	
	/**
 	 * The matcher fails.
	 */
	RET_FALSE,

	/**
	 * Unconditional jump.
	 */
	GOTO                
};


struct OpNode
{
	uint8_t op;
	uint16_t operandLen;
	uint16_t ifTrue;
	uint16_t ifFalse;
	union
	{
		double number;
		const char* string;
		uint32_t code;			// TODO: uint16_t?
	};
};


struct Operation
{
	uint8_t op;
	uint16_t operandLen;
	Operation* ifTrue;
	Operation* ifFalse;
	union
	{
		double number;
		const char* string;
		uint32_t code;			// TODO: uint16_t?
	};
	Operation* caller;

};


class QueryParser : public Parser
{
public:
	QueryParser(FeatureStore* store, const char* pInput) :
		Parser(pInput),
		store_(store)
	{
	}

private:
	uint32_t getStringCode(const char* str, int len)
	{
		return store_->strings().getCode(str, len);
	}

	std::vector<OpNode> ops_;
	FeatureStore* store_;
	int globalStringNo_;		// global string code of "no"
};
