// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#include "MatcherCompiler.h"
#include "Matcher.h"
#include "MatcherDecoder.h"
#include "MatcherEngine.h"
#include "MatcherEmitter.h"
#include "MatcherParser.h"
#include "MatcherValidator.h"
#include <common/util/BufferWriter.h>
#include <common/util/log.h>

const MatcherHolder* MatcherCompiler::getMatcher(const char* query)
{
	MatcherParser parser(store_, query);
	Selector* sel = parser.parse();
	uint32_t indexBits = parser.indexBits();  // TODO
	const MatcherHolder* matcher = nullptr;

	// OpNode* node = graph->root();
	// assert(node->opcode == Opcode::FEATURE_TYPE);

	if (sel->next == nullptr)
	{
		// Single-selector query
		FeatureTypes types = sel->acceptedTypes;
		TagClause* clause = sel->firstClause;
		if (clause == nullptr)
		{
			// Types only
			matcher = MatcherHolder::createMatchAll(types);
		}
		else
		{
			OpNode* keyOp = &clause->keyOp;
			if (clause->next == nullptr &&
				keyOp->opcode == Opcode::GLOBAL_KEY &&
				!keyOp->isNegated())
			{
				// single positive global-key op
				int globalKeyCode = keyOp->operand.code;
				OpNode* valueOp = keyOp->next[1];
				assert(valueOp);
				if (valueOp->opcode == Opcode::EQ_CODE && 
					valueOp->next[0]->opcode == Opcode::RETURN &&
					valueOp->next[1]->opcode == Opcode::RETURN)
				{
					// single EQ_CODE value-op

					int valueCode = valueOp->operand.code;
					if (valueOp->isNegated())
					{
						int codeNo = parser.codeNo();
						if (valueCode == codeNo)
						{
							matcher = MatcherHolder::createMatchKey(
								types, indexBits, globalKeyCode, codeNo);
						}
					}
					else
					{
						matcher = MatcherHolder::createMatchKeyValue(
							types, indexBits, globalKeyCode, valueCode);
					}
				}
			}
		}
	}
	if (!matcher)
	{
		matcher = compileMatcher(parser.graph(), sel, indexBits);
#ifdef _DEBUG
		DynamicBuffer buf(1024);
		BufferWriter out(&buf);
		MatcherDecoder decoder(store_, out,
			reinterpret_cast<const uint16_t*>(
				reinterpret_cast<const uint8_t*>(&matcher->mainMatcher_) + sizeof(Matcher)));
		decoder.decode();
		LOG("%.*s\n", static_cast<int>(buf.length()), buf.data());
#endif
	}
	matcher->addref();
	return matcher;
}

const MatcherHolder* MatcherCompiler::compileMatcher(OpGraph& graph, Selector* firstSel, uint32_t indexBits)
{
	MatcherValidator validator(graph);
	OpNode* root = validator.validate(firstSel);

	size_t resourceSize = validator.resourceSize();
	size_t matcherSize = sizeof(MatcherHolder) + resourceSize + validator.maxInstructionSize();
	uint8_t* matcherData = new uint8_t[matcherSize];
	MatcherHolder* matcherHolder = reinterpret_cast<MatcherHolder*>(matcherData + resourceSize);
	uint16_t* pCode = reinterpret_cast<uint16_t*>(matcherData + resourceSize + sizeof(MatcherHolder));
	
	new (matcherHolder)MatcherHolder(validator.featureTypes(), indexBits, indexBits==0 ? 0 : 1);
		// TODO: Specifying 1 for keyMin means that index buckets are selected if they
		// contain *any* of the keys in the query; some queries may require *all* keys
		// to be present, in which case setting keyMin to keyMask would result in 
		// more effective index use -- however, these cases are rare (e.g. "find hotels
		// with a restaurant na[tourism=hotel][amenity=restaurant]")
	matcherHolder->resourcesLength_ = resourceSize;
	matcherHolder->regexCount_ = validator.regexCount();

	MatcherEmitter emitter(graph, root, matcherData, pCode);
	emitter.emit();
	emitter.fixJumps();

	new (&matcherHolder->mainMatcher_)Matcher((MatcherMethod)MatcherEngine::accept, store_);

	return matcherHolder;
}
