// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once
#include "OpGraph.h"

struct TagClause
{
public:
	TagClause(int keyCode, int category);
	TagClause(std::string_view key);

	enum Flags
	{
		/* The 7 value flags indicate which kinds of values are examined
	   by a clause. These are set for both value-ops and key-ops.
	   EQ_CODE sets VALUE_GLOBAL_STRING
	   EQ_STR  sets VALUE_LOCAL_STRING
	   STARTS_WITH/ENDS_WITH/CONTAINS set VALUE_ANY_STRING if the match
	   string can be used on a number, otherwise VALUE_TEXT_STRING
	   (which means a value-op/clause only applies to tag vlaues that
		are global or local strings)
	   Numeric ops set VALUE_ANY_NUMBER, unless value-op is ="123",
	   (which only matches a specific literal number)
	   */
		VALUE_GLOBAL_STRING = 1 << 1,
		VALUE_LOCAL_STRING = 1 << 2,
		VALUE_NARROW_NUMBER = 1 << 3,
		VALUE_WIDE_NUMBER = 1 << 4,
		VALUE_TEXT_STRING = 1 << 5,
		VALUE_ANY_STRING = 1 << 6,
		VALUE_ANY_NUMBER = 1 << 7,

		/**
		 * Used on key-ops to indicate that the clause uses nested AND/OR
		 * expressions (rare)
		 */
		COMPLEX_BOOLEAN_CLAUSE = 1 << 8,
	};

	bool isOrClause() const;
	OpNode** insertValueOp(OpNode* node, bool asAnd);
	void absorb(TagClause* other);
	void insertLoadOps();

	static const int OPCODE_VALUE_TYPES[];

	TagClause* next;
	int category;
	uint32_t flags;
	OpNode keyOp;
	OpNode trueOp;
};
