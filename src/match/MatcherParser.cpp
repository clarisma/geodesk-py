// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#include "MatcherParser.h"


MatcherParser::MatcherParser(FeatureStore* store, const char* pInput) :
	Parser(pInput),
	store_(store),
	indexBits_()
{
	codeNo_ = getStringCode(std::string_view("no", 2));
}

Selector* MatcherParser::parse()
{
	Selector* firstSel = nullptr;
	Selector** pNextSel = &firstSel;
	for (;;)
	{
		Selector* sel = expectSelector();
		*pNextSel = sel;
		pNextSel = &sel->next;
		if (*pNext_ != ',') break;
		pNext_++;
		skipWhitespace();
	}
	if (*pNext_ != 0) error("Expected [ or ,");
	return firstSel;	
}

Selector* MatcherParser::expectSelector()
{
	FeatureTypes types = matchTypes();
	if (types == 0)
	{
		types = FeatureTypes::ALL;
		if (*pNext_ != '[') error("Expected selector");
	}
	Selector* sel = graph_.arena().alloc<Selector>();
	new(sel) Selector(types);
	currentSel_ = sel; 
	while (accept('['))
	{
		TagClause* clause = expectTagClause();
		expect(']');
		sel->addClause(clause);
		indexBits_ |= sel->indexBits; // TODO
	}
	return sel;
}


TagClause* MatcherParser::expectTagClause()
{
	TagClause* clause;
	OpNode* valueOp;
	if (accept('!'))
	{
		clause = expectKey();
		valueOp = graph_.newOp(Opcode::EQ_CODE, codeNo_);
		clause->keyOp.setNegated(true);
		clause->insertValueOp(valueOp, true);
		return clause;
	}

	clause = expectKey();
	
	int opcode;
	bool negated = false;
	bool acceptString = false;
	switch (*pNext_)
	{
	case '=':
		pNext_++;
		if (*pNext_ == '=') pNext_++;
		opcode = Opcode::EQ_CODE;
		acceptString = true;
		break;
	case '>':
		pNext_++;
		if (*pNext_ == '=')
		{
			opcode = Opcode::GE;
			pNext_++;
		}
		else
		{
			opcode = Opcode::GT;
		}
		break;
	case '<':
		pNext_++;
		if (*pNext_ == '=')
		{
			opcode = Opcode::LE;
			pNext_++;
		}
		else
		{
			opcode = Opcode::LT;
		}
		break;
	case '!':
		negated = true;
		switch (*(pNext_ + 1))
		{
		case '=':
			opcode = Opcode::EQ_CODE;
			acceptString = true;
			break;
		case '~':
			opcode = Opcode::REGEX;
			acceptString = true;
			break;
		default:
			error("Expected != or !~");
		}
		pNext_ += 2;
		break;
	case '~':
		opcode = Opcode::REGEX;
		pNext_++;
		acceptString = true;
		break;
	default:	
		valueOp = graph_.newOp(Opcode::EQ_CODE, codeNo_);
		valueOp->setNegated(true);
		clause->insertValueOp(valueOp, false);
		clause->flags |= TagClause::KEY_REQUIRED;
		return clause;
	}

	skipWhitespace();

	clause->flags |= negated ? 0 : TagClause::KEY_REQUIRED;
	clause->keyOp.setNegated(negated);
	if (acceptString)
	{
		for (;;)
		{
			if (opcode == Opcode::REGEX)
			{
				valueOp = graph_.newOp(opcode, expectRegex());
			}
			else
			{
				valueOp = acceptStringOperand();
				if (!valueOp)
				{
					double num = number();
					if (std::isnan(num))
					{
						error("Expected string or number");
					}
					valueOp = graph_.newOp(Opcode::EQ_NUM, num);
				}
			}
			valueOp->setNegated(negated);
			clause->insertValueOp(valueOp, negated);
			if (!accept(',')) break;
		}
	}
	else
	{
		// accept only number
		double num = number();
		if (std::isnan(num)) error("Expected number");
		valueOp = graph_.newOp(opcode, num);
		valueOp->setNegated(negated);
		clause->insertValueOp(valueOp, negated);
	}
	return clause;
}


FeatureTypes MatcherParser::matchTypes()
{
	FeatureTypes types = 0;
	if (*pNext_ == '*')
	{
		types = FeatureTypes::ALL;
		pNext_++;
	}
	else
	{
		/*
		static const char typeChars[] = "nwar";
		static const FeatureTypes typeMasks[] =
		{
			FeatureTypes::NODES,
			FeatureTypes::NONAREA_WAYS,
			FeatureTypes::AREAS,
			FeatureTypes::NONAREA_RELATIONS,
		};
		int found = 0;
		*/
		for (;;)
		{
			char ch = *pNext_;
			FeatureTypes t = 0;
			if (ch == 'n')
			{
				t = FeatureTypes::NODES;
			}
			else if (ch == 'w')
			{
				t = FeatureTypes::NONAREA_WAYS;
			}
			else if (ch == 'a')
			{
				t = FeatureTypes::AREAS;
			}
			else if (ch == 'r')
			{
				t = FeatureTypes::NONAREA_RELATIONS;
			}
			else
			{
				break;
			}
			if (types & t)
			{
				error("Type '%c' specified more than once", ch);
				return 0;
			}
			types |= t;
			pNext_++;
		}
	}
	skipWhitespace();
	return types;
}


const CharSchema MatcherParser::VALID_FIRST_CHAR
{
	0b0000010000000000000000000000000000000000000000000000000000000000,
	0b0000011111111111111111111111111010000111111111111111111111111110,
	0b1111111111111111111111111111111111111111111111111111111111111111,
	0b1111111111111111111111111111111111111111111111111111111111111111
};

const CharSchema MatcherParser::VALID_NEXT_CHAR
{
	0b0000011111111111000000000000000000000000000000000000000000000000,
	0b0000011111111111111111111111111010000111111111111111111111111110,
	0b1111111111111111111111111111111111111111111111111111111111111111,
	0b1111111111111111111111111111111111111111111111111111111111111111,
};


std::string_view MatcherParser::acceptEscapedString()
{
	ParsedString parsed = string();
	// TODO: If strign has escape characters, allocate an unescaped copy in the arena
	return parsed.asStringView();
}

RegexOperand* MatcherParser::expectRegex()
{
	const char* pCurrent = pNext_;
	ParsedString parsed = string();
	if (parsed.isNull()) error("Expected regex");
	try
	{
		return graph_.addRegex(parsed.str, parsed.len);
	}
	catch (const std::regex_error& e)
	{
		pNext_ = pCurrent;		// TODO: mark entire regex as error
		error(e.what());		// throws
		return nullptr;			// not needed, just to suppress warning
	}
}

TagClause* MatcherParser::expectKey()
{
	// Keys are allowed to start with a number (e.g. "4wd")
	std::string_view key = identifier(VALID_NEXT_CHAR, VALID_NEXT_CHAR);
	if (key.empty())
	{
		key = acceptEscapedString();
		if (key.empty()) error("Expected key");		// throws
	}
	TagClause* clause = graph_.arena().alloc<TagClause>();
	int code = getStringCode(key);
	if (code > 0)
	{
		int category = store_->getIndexCategory(code);
		new(clause)TagClause(code, category);
	}
	else
	{
		new(clause)TagClause(key);
	}
	clause->keyOp.next[0] = &currentSel_->falseOp;
	return clause;
}

/**
 * Checks for a string operand and returns an OpNode with the proper 
 * opcode and operand data. If the operand is preceded and/or followed
 * by asteriscs, uses opcode STARTS_WITH, ENDS_WITH or CONTAINS, 
 * otherwise EQ_STR.
 * If next token is not a string, returns NULL.
 * 
 * pNext_ will be positioned at the start of the next token.
 */
OpNode* MatcherParser::acceptStringOperand()
{
	int op = Opcode::EQ_STR;
	std::string_view val;
	if (*pNext_ == '*')
	{
		pNext_++;
		op = Opcode::ENDS_WITH;
		val = identifier(VALID_FIRST_CHAR, VALID_NEXT_CHAR);
		if (val.empty())
		{
			op = Opcode::CONTAINS;
		}
		else if (*pNext_ == '*')
		{
			pNext_++;
			op = Opcode::CONTAINS;
		}
	}
	else
	{
		val = identifier(VALID_FIRST_CHAR, VALID_NEXT_CHAR);
		if (!val.empty())
		{
			if (*pNext_ == '*')
			{
				pNext_++;
				op = Opcode::STARTS_WITH;
			}
		}
		else
		{
			val = acceptEscapedString();
			if (val.data() == nullptr) return nullptr;
			
			const char* s = val.data();
			size_t len = val.length();
			if (len > 0)
			{
				if (s[0] == '*')
				{
					s++;
					len--;
					if (len > 0 && s[len-1] == '*')
					{
						len--;
						op = Opcode::CONTAINS;
					}
					else
					{
						op = Opcode::ENDS_WITH;
					}
				}
				else if (s[len-1] == '*')
				{
					len--;
					op = Opcode::STARTS_WITH;
				}
				val = std::string_view(s, len);
			}
		}
	}

	OpNode* node;
	int code;
	if (op == Opcode::EQ_STR && (code = getStringCode(val)) > 0)
	{
		node = graph_.newOp(Opcode::EQ_CODE, code);
	}
	else
	{
		node = graph_.newOp(op, val);
	}
	skipWhitespace();
	return node;
}


