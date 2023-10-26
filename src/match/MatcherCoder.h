// Copyright (c) 2023 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

// #define ASMJIT_STATIC 
#include <asmjit/asmjit.h>
#include "ast.h"

class Matcher;

using namespace asmjit;

enum TrailingClauses
{
	NONE = 0,
	REQUIRED = 1,
	ONLY_OPTIONAL = 2
};

class MatcherCoder
{
public:
	MatcherCoder(JitRuntime &rt);

	const Matcher* createMatcher(Selector* sel);

private:
	CodeHolder code_;
	x86::Assembler a_;

	/**
	 * Writes code to execute a single Selector. 
	 * 
	 * @param sel               the Selector
	 * @param checkLocalKeyFlag if true, code should be generated to perform
	 *                          a fast check based on the local-key flag if the
	 *                          Selector has certain local-key clauses 
	 */
	void selector(const Selector* sel, const Label& success);
	void matchClause(const TagClause* clause, bool jumpIfTrue, const Label& target);
	void scanGlobalKeys(const TagClause* clause);

	/**
	 * Writes code to check if a candidate string matches the given test
	 * string.
	 * 
	 * @param ptrReg	   the 64-bit register that holds the pointer to 
	 *                     the candidate string 
	 * @param str          the string to compare against
     * @param strLen       the length of `str`
     * @param matchLen     If true, the candidate string's length must 
     *                     be `strLen` (false means ptrReg points to a substring)
     * @param jumpIfMatch  If true, the generated code will branch if the strings
     *                     match; otherwise, the code will branch if not matched
     * @param label        the label to jump to 
     */
	void matchString(x86::Gp ptrReg, const char* str, uint32_t strLen,
		bool matchLen, bool jumpIfMatch, Label label);

	/**
	 * The register where the pointer to the current tag is stored.
	 * (RDX on windows, RSI on Linux)
	 */
	x86::Gp tagPtr_;
	
	/**
	 * The register where the tag-table pointer is stored (R8).
	 */
	x86::Gp tagTablePtr_;

	/**
	 * Indicates whether the matcher relies on the local-key flag
	 * (if so, the flag is stored as bit 0 of tagTablePtr_)
	 */
	bool localKeyFlagUsed_;

	/**
	 * What kinds of clauses follow the current one: NONE, REQUIRED,
	* or ONLY_OPTIONAL.
	*/
	TrailingClauses trailingClauses_;

};
