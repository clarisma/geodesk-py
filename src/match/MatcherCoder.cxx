// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

﻿#include "MatcherCoder.h"

MatcherCoder::MatcherCoder(JitRuntime& rt) :
	a_(nullptr),
	tagTablePtr_(x86::r8)
{
	code_.init(rt.environment(), rt.cpuFeatures());
	code_.attach(&a_);
}


const Matcher* MatcherCoder::createMatcher(Selector* firstSel)
{
	// The local-keys flag gives us a shortcut to determine if certain
	// Selectors will succeed or fail, without having to scan the tags.
	//
	// A) If a Selector contains *only* optional-local-key clauses,
	//    (e.g. [!uncommon_key] or [uncommon_key!=value]), the
	//    Selector always SUCCEEDS if the local-keys flag is false.
	//
	// B) If a Selector contains *any* required-local-key clauses
	//    (e.g. [uncommon_key] or [uncommon_key=value]), the
	//    Selector always FAILS if the local-keys flag is false.
	//
	// If the Query has multiple Selectors:
	//
	// - If either A or B applies to all, we can perform the local-key
	//   flag check upfront
	// - If A or B applies to none, we ignore the local-key flag
	// - Otherwise, we need to check the local-key flag for each
	//   Selector for which A or B applies

	bool anyLocalKeys = false;
	bool localKeyFlagUsed_ = false;
	bool queryTrueIfNoLocals = true;
	bool queryFalseIfNoLocals = true;

	Selector* sel = firstSel;
	while (sel)
	{
		int ct = sel->clauseTypes;
		if ((ct & (Selector::CLAUSE_LOCAL_OPTIONAL |
			Selector::CLAUSE_LOCAL_REQUIRED)) != 0)
		{
			anyLocalKeys = true;
			localKeyFlagUsed_ = true;
		}
		if (ct != Selector::CLAUSE_LOCAL_OPTIONAL)
		{
			queryTrueIfNoLocals = false;
		}
		if ((ct & Selector::CLAUSE_LOCAL_REQUIRED) == 0)
		{
			queryFalseIfNoLocals = false;
		}
		sel = sel->next;
	}

	// If there are multiple selectors, or there are local-key 
	// clauses in a single selector, we will need the tag-table 
	// pointer more than once and hence have to stash a copy
	bool mustSaveTagTablePtr = firstSel->next != nullptr ||
		(firstSel->clauseTypes &
			(Selector::CLAUSE_LOCAL_OPTIONAL |
				Selector::CLAUSE_LOCAL_REQUIRED));

	// The pointer to the feature is passed as an argument to the
	// Matcher function: RDX on Windows, RSI on Linux
	// Since we only need this pointer to obtain the tag-table pointer
	// (and the local-key flag), we can re-use this register to store
	// the pointer to the current tag.
	x86::Gp featurePtr;
#ifdef _WIN32
	featurePtr = x86::rdx;
#else
	featurePtr = x86::rsi;
#endif
	// Obtain the tag-table pointer (including the local-keys flag
	// stored in bit 0)
	tagPtr_ = featurePtr;
	a_.movsx(x86::rax, x86::dword_ptr(featurePtr, 8));
	a_.add(tagPtr_, x86::rax);

	if (queryTrueIfNoLocals || queryFalseIfNoLocals)
	{
		// If all selectors have *only* optional-local-key clauses
		// (e.g. [!uncommon_key] or [uncommon_key!=value]), the
		// Matcher always SUCCEEDS if the local-keys flag is false.
		// If all selectors have *any* required-local-key clauses
		// (e.g. [common][uncommon] or [uncommon=value]), the
		// Matcher always FAILS if the local-keys flag is false.

		// TODO: could optimize, local-key flag is rare, forward-jumps
		// are typically not taken; so better to jump if flag NOT set

		Label must_check_selectors = a_.newLabel();
		// If the local-key flag is set, we have to do a full 
		// tag-table check
		a_.btr(tagPtr_, 0);		// check & clear local-key flag 
		a_.jc(must_check_selectors);	// jump if set
		// Otherwise, we can shortcut
		if (queryTrueIfNoLocals)
		{
			a_.mov(x86::eax, 1);	// return true
		}
		else
		{
			a_.xor_(x86::eax, x86::eax);  // return false
		}
		a_.ret();
		a_.bind(must_check_selectors);
		// Since we're checking the local-keys flag upfront,
		// we don't have to check it for each individual Selector
		localKeyFlagUsed_ = false;
	}

	if (!localKeyFlagUsed_)
	{
		a_.and_(tagPtr_, -2);	// strip off the local-key flag
		// (for all uses)
	}
	if (mustSaveTagTablePtr)
	{
		// If we need the tag-table pointer again (because there are multiple
		// selectors, or the single selector contains local-key clauses), 
		// stash it for later (with or without local-key flag)
		a_.mov(tagTablePtr_, tagPtr_);
	}

	Label matcher_success = a_.newLabel();

	sel = firstSel;
	for (; ; )
	{
		selector(sel, matcher_success);
		sel = sel->next;
		if (!sel) break;
		// If there are more selectors, reset tagPtr_ to start of tagtable
		// (preserve the local-key flag if it is used)
		a_.mov(tagPtr_, tagTablePtr_);
	}

	// none of the selectors matched: return false
	a_.xor_(x86::eax, x86::eax);
	a_.ret();

	// at least one selector matched: return true
	a_.bind(matcher_success);
	a_.mov(x86::eax, 1);
	a_.ret();
	

	return nullptr; // TODO
}


/**
 * Emits code for a single selector.
 */
// Prior to call:
// - tagPtr_ must contain the pointer to the tag table
// - If localKeyFlagUsed_ is true, tagPtr_ must contain the 
//   local-key flag in bit 0
// - If local-key clauses are present, tagTablePtr_ must be valid

void MatcherCoder::selector(const Selector* sel, const Label& success)
{
	// TODO: these should be class members?
	Label selector_failed = a_.newLabel();

	int ct = sel->clauseTypes;

	if (localKeyFlagUsed_)
	{
		bool localKeyFlagCleared = false;
		if (ct == Selector::CLAUSE_LOCAL_OPTIONAL)
		{
			// If this Selector *only* has optional-local-key clauses,
			// it will always succeed if the local-key flag is false
			// (in this case, the whose Matcher is successful)

			a_.btr(tagPtr_, 0);		// check & clear local-key flag 
			a_.jnc(success);
			localKeyFlagCleared = true;
		}
		else if ((ct & Selector::CLAUSE_LOCAL_REQUIRED) != 0)
		{
			// If this Selector has *any* required-local-key clauses,
			// it will always fail if the local-key flag is false
			// (In this case, we skip the Selector)

			a_.btr(tagPtr_, 0);		// check & clear local-key flag 
			a_.jnc(selector_failed);
			localKeyFlagCleared = true;
		}
		if (!localKeyFlagCleared)
		{
			// If we haven't generated code that clears the local-key flag,
			// generate it now
			a_.and_(tagPtr_, -2);
		}
	}
	// If localKeyFlagUsed_ is false, we're guaranteed that tagPtr_
	// will not have bit 0 set

	const TagClause* clause = sel->firstClause;
	if (clause->isGlobalKey())
	{
		while (clause)
		{
			if (clause->isLocalKey()) break;
			matchClause(clause, true, success);	// TODO: fix
			clause = clause->next;
		}
		if (clause)
		{
			// local-key clauses follow

			// If we hit the end of the global-key section of the tag-table
			// while checking a clause, and only optional clauses followed,
			// execution jumps to this intermediate success-point

			a_.bind(selector_success);
			selector_success = a_.newLabel();

			a_.mov(tagPtr_, tagTablePtr_);

			if ((ct & (Selector::CLAUSE_LOCAL_REQUIRED
				| Selector::CLAUSE_LOCAL_OPTIONAL))
				== Selector::CLAUSE_LOCAL_OPTIONAL)
			{
				// If the Selector has only optional local-key clauses,
				// check if there are no local keys present; if so,
				// the Selector succeeds at this point

				a_.btr(tagPtr_, 0);		// check & clear the local key-flag
				a_.jnc(selector_success);
			}
			else
			{
				a_.and_(tagPtr_, -2);	// clear the local-key flag
			}
			// set tag pointer to first local key
			a_.sub(tagPtr_, 4);
		}
	}
	else
	{
		// If the selector only checks local keys,
		// move tag pointer to the first local key,
		// stored directly ahead of the table start
		a_.sub(tagPtr_, 4);
	}

	while (clause)
	{
		matchClause();
		clause = clause->next;
	}

	// matched all clauses, return true
	a_.bind(selector_success);
	a_.mov(x86::al, 1);
	a_.ret();

	// common exit point when we've failed to match a clause
	a_.bind(selector_failed);

}



/**
 * Emits code to check if a global key possibly exists in the Tag Table.
 *
 * - The tagPtr_ register must contain the address of the first 
 *   global key to check (not necessarily the first in the table)
 * - The generated code keeps advancing tagPtr_ until it finds a tag
 *   whose key is equal or above the key code of the current clause
 * - AX will contain the key code and flags; the upper half of EAX
 *   will contain the narrow tag value
 * 
 * It is up to the caller to actually check if the global key has been
 * found.
 *
 * @param clause the TagClause whose global key to check
 */
void MatcherCoder::scanGlobalKeys(const TagClause* clause)
{
	uint16_t keyBits = clause->keyCode << 2;
	Label check_next_tag = a_.newLabel();
	Label matched_key_or_end = a_.newLabel();
	// Load the current tag
	a_.mov(x86::eax, x86::ptr(tagPtr_));
	a_.cmp(x86::eax, keyBits);
	a_.jnb(matched_key_or_end);
	a_.bind(check_next_tag);
	a_.and_(x86::eax, 2);
	a_.lea(tagPtr_, x86::ptr(tagPtr_, x86::rax, 1, 4));
	a_.mov(x86::eax, x86::ptr(tagPtr_));
	a_.cmp(x86::eax, keyBits);
	a_.jb(check_next_tag);
	a_.bind(matched_key_or_end);

}


/**
 * Emits code to match a Tag Clause.
 *
 * For a required-key clause to succeed:
 *
 * - Its key must be found
 * - If the clause is followed by other required-key clauses, the tag
 *   cannot be the last (otherwise, the other clauses will fail)
 * - If the clause only accepts a single value type, and this type is
 *   global string or local string, the tag's value must be of this type
 *   (see exception below for explicitly required keys)
 * - If the key is explicitly required (but not implicitly), its value
 *   cannot be "no" (E.g. [bridge] or [bridge][bridge!=retractable]);
 *   if only accepted value is local string, the clause implicitly accepts
 *   global string as well (since "no" is always a global string), and
 *   we cannot fail early
 * - The clause expression (if any) must be true
 *
 * An optional-key clause succeeds if:
 *
 * - Its key is not found; OR
 * - If the clause has an expression:
 * 		- If it only accepts a single value type, AND this type is global
 * 	 	  string or local string, AND the tag's value is of another type; OR
 * 	 	- The expression is true
 * - If it has no expression:
 * 		- the tag's value type is not global string; OR
 *      - the tag's value is global string "no"
 *
 * However, an optional-key clause fails in any case if it is followed by
 * required-key clause, and the tag is the last (which means the required-key
 * clause has no chance of matching).
 *
 * If the clause does not match, the code branches to selector_failed
 * Otherwise:
 * - If the clause is followed only by optional-key clauses (of the same
 *   type), and this tag is the last, the code branches to selector_success
 * Otherwise, the code continues. If there are further clauses of the same
 * type, $pos is moved to the next key in the table.
 *
 * Required fields: clause
 */
void MatcherCoder::matchClause(const TagClause* clause, bool jumpIfTrue, const Label& target)
{
	Label matched_clause = a_.newLabel();
	Label matched_clause_no_key = a_.newLabel();

	// Determine the current clause's trailing clauses
	// of the same local/global type

	bool isLocalKey = clause->isLocalKey();
	const TagClause* sibling = clause->next;
	trailingClauses_ = TrailingClauses::NONE;
	while (sibling)
	{
		if (sibling->isLocalKey() && !isLocalKey) break;
		trailingClauses_ = TrailingClauses::ONLY_OPTIONAL;
		if (sibling->isKeyRequired())
		{
			trailingClauses_ = TrailingClauses::REQUIRED;
			break;
		}
		sibling = sibling->next;
	}

	bool failIfLast = trailingClauses_ == TrailingClauses::REQUIRED;

	// Let's see if we can match or fail based solely on the type of the
	// tag value. If the clause accepts a) only global strings or b) only
	// local strings, a required-key clause will fail (and an optional-key
	// clause will succeed) if the tag value is of any other type
	// If a key is explicitly required, but not implicitly, we'll also
	// have to examine if the value is "no" (always a global string)

	bool requiredExplicitlyOnly =
		((clause_->flags & (TagClause::KEY_REQUIRED_EXPLICITLY |
			TagClause::KEY_REQUIRED_IMPLICITLY)) ==
			TagClause::KEY_REQUIRED_EXPLICITLY);
	// Match clause if key present unless it has certain value(s)

	bool failIfWrongType;
	bool matchedIfWrongType;

	if (requiredExplicitlyOnly ||
		(!clause_->isKeyRequired() && clause.expression() != null))
	{
		// [k], [k!=v] and [k][k!=v] succeed if wrong type
		matchedIfWrongType = true;
		failIfWrongType = false;
	}
	else
	{
		// [!k], [k=v] and [k][k=v] fail if wrong type
		matchedIfWrongType = false;
		failIfWrongType = true;
	}

	int clauseValueTypes = clause.flags() & TagClause.VALUE_ANY;
	/*
	if(requiredExplicitlyOnly || clause.expression() == null)
	{
		// [k], [!]k and [k][k!=v] require "no" check, therefore
		// need to look at global string value
		clauseValueTypes |= TagClause.VALUE_GLOBAL_STRING;
	}
	*/
	if (clauseValueTypes == TagClause.VALUE_GLOBAL_STRING)
	{
		acceptedTagType = TagValues.GLOBAL_STRING;
	}
	else if (clauseValueTypes == TagClause.VALUE_LOCAL_STRING)
	{
		acceptedTagType = TagValues.LOCAL_STRING;
	}
	else
	{
		acceptedTagType = MULTIPLE_TAG_TYPES;
		matchedIfWrongType = false;
		failIfWrongType = false;
	}

	boolean advancePos = trailingClauses != NONE;
	int advanceBy = 0;


	if (isLocalKey)
	{
		scanLocalKeys(matched_clause_no_key);

		// At this point in the generated code, we've found the local key
	}
	else
	{
		scanGlobalKeys();
		mv.visitVarInsn(ILOAD, $tag);
		if (matchGlobalKeyGlobalStringValue())
		{
			// We can do simple & fast [ns=ns] check

			skipOptionalClauses();
			if (trailingClauses != NONE) mv.visitIincInsn($pos, 4);
			return;
		}

		// Otherwise, check key and possibly type. For required-key
		// clauses, we can simultaneously perform a check for the
		// last-entry flag, as well

		int key = clause.keyCode();
		if (clause.isKeyRequired())
		{
			int checkBits = key << 2;
			int maskBits = 0x7ffc;
			if (failIfLast)
			{
				maskBits |= 0x8000;
				failIfLast = false;
			}
			if (failIfWrongType)
			{
				maskBits |= 3;
				checkBits |= acceptedTagType;
				failIfWrongType = false;
			}
			if (maskBits == 0xffff)
			{
				mv.visitInsn(I2C);	// convert to char is same as AND 0xffff
			}
			else
			{
				loadIntConstant(maskBits);
				mv.visitInsn(IAND);
			}
			loadIntConstant(checkBits);
			mv.visitJumpInsn(IF_ICMPNE, selector_failed); // ──────⯈╌╌╌╌╌╌╌
		}
		else
		{
			// If we don't match key, optional clause succeeds, but we don't advance $pos
			loadIntConstant(0x7ffc);
			mv.visitInsn(IAND);
			loadIntConstant(key << 2);
			mv.visitJumpInsn(IF_ICMPNE, matched_clause_no_key); // ──⯈──╮
		}

		if (clause.expression() == null)
		{
			// [k] or [!k]: Check for "no"
			mv.visitVarInsn(ILOAD, $tag);
			if (trailingClauses != REQUIRED)
			{
				loadIntConstant(0xffff_7fff);
				mv.visitInsn(IAND);
			}
			loadIntConstant((key << 2) | 1 | (valueNo << 16));
			if (clause.isKeyRequired())
			{
				// [k] fails of value = "no"
				mv.visitJumpInsn(IF_ICMPEQ, selector_failed); // ───⯈╌╌╌╌╌╌
				matchedIfWrongType = false;
			}
			else
			{
				// [!k] fails UNLESS value = "no"
				mv.visitJumpInsn(IF_ICMPNE, selector_failed); // ───⯈╌╌╌╌╌╌
				// At this point, we don't have to check the last-entry
				// and type flags, since we've done so implicitly
				// Since the only possible matching value is value="no",
				// the advanceBy value can only be 4
				failIfLast = false;
				failIfWrongType = false;
				advanceBy = 4;
			}
		}
	}

	if (failIfWrongType)
	{
		int maskBits = 3;
		if (failIfLast)
		{
			maskBits |= isLocalKey ? 4 : 0x8000;
			// TODO: should not process global keys here, so can only be 4
			failIfLast = false;
		}
		mv.visitVarInsn(ILOAD, $tag);
		loadIntConstant(maskBits);
		mv.visitInsn(IAND);
		loadIntConstant(acceptedTagType);
		mv.visitJumpInsn(IF_ICMPNE, selector_failed); // ───⯈╌╌╌╌╌╌╌╌╌╌╌╌╌╌
	}
	if (failIfLast)
	{
		// If the clause fails if the matched tag is at end of table,
		// and we haven't encoded a check yet, generate it now:
		// Isolate the flag (Bit 2 for local, bit 15 for global),
		// and fail the selector if the flag is set

		mv.visitVarInsn(ILOAD, $tag);
		loadIntConstant(isLocalKey ? 4 : 0x8000);
		mv.visitInsn(IAND);
		mv.visitJumpInsn(IFNE, selector_failed); // ───⯈╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
	}

	if (matchedIfWrongType)
	{
		mv.visitVarInsn(ILOAD, $tag);
		mv.visitInsn(ICONST_3);
		mv.visitInsn(IAND);
		loadIntConstant(acceptedTagType);
		mv.visitJumpInsn(IF_ICMPNE, matched_clause); // ──⯈──╮
	}

	if (clause.expression() != null)
	{
		// Clauses other than [k] or [!k]
		checkExpression();                           // ───⯈╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
		if (requiredExplicitlyOnly)
		{
			// [k][k!=v] fails if value is "no"
			mv.visitVarInsn(ILOAD, $val_global_string);
			loadIntConstant(valueNo);
			mv.visitJumpInsn(IF_ICMPEQ, selector_failed); // ───⯈╌╌╌╌╌╌╌╌╌╌
		}
		advancePos = false; // because checkExpression already did it
	}
	else
	{
		if (isLocalKey)
		{
			// For local-key [k] and [!k], check for "no" here

			// TODO: check???

			loadLocalTagNarrowValue();
			loadIntConstant(valueNo);
			if (clause.isKeyRequired())
			{
				mv.visitJumpInsn(IF_ICMPEQ, selector_failed); // ───⯈╌╌╌╌╌╌
			}
			else
			{
				mv.visitJumpInsn(IF_ICMPNE, selector_failed); // ───⯈╌╌╌╌╌╌
				advanceBy = -6;
			}
		}
	}

	mv.visitLabel(matched_clause); // ⯇──────────────────────╯
	skipOptionalClauses();
	if (advancePos)
	{
		// If more clauses follow, we need to advance $pos

		// TODO: wrong, might have advanced already in checkExpression

		if (advanceBy != 0)
		{
			mv.visitIincInsn($pos, advanceBy);
		}
		else
		{
			if (isLocalKey)
			{
				nextLocalKeyTag();
			}
			else
			{
				nextGlobalKeyTag();
			}
		}
	}

	// We jump here if the clause matched even if we didn't find a tag
	// [!k] or [k!=v]
	mv.visitLabel(matched_clause_no_key); // ⯇──────────────────────────╯

}



void MatcherCoder::matchString(x86::Gp ptrReg, const char* str,
	uint32_t strLen, bool matchLen, bool jumpIfMatch, Label label)
{
	// TODO

	x86::Gp workReg1 = x86::r10;
	x86::Gp workReg2 = x86::r11;
	
	int32_t ofs;
	uint64_t run;
	a_.mov(workReg1, ptr(ptrReg, ofs));
	a_.xor_(workReg1, run);
}
