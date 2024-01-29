// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#include "Matcher.h"
#include <cstddef>   // for offsetof
#include <regex>

const Matcher* MatcherHolder::defaultRoleMethod(const RoleMatcher* matcher, const uint8_t*)
{
	return (const Matcher*)matcher + (offsetof(MatcherHolder, mainMatcher_) - 
		offsetof(MatcherHolder, defaultRoleMatcher_));
}

bool MatcherHolder::matchAllMethod(const Matcher*, const uint8_t*)
{
	return true;
}


MatcherHolder::MatcherHolder(FeatureTypes types, uint32_t keyMask, uint32_t keyMin) :
	refcount_(1),
	acceptedTypes_(types),
	resourcesLength_(0),
	referencedMatcherHoldersCount_(0),
	regexCount_(0),
	roleMatcherOffset_(offsetof(MatcherHolder, defaultRoleMatcher_)),
	defaultRoleMatcher_(defaultRoleMethod, nullptr),
	mainMatcher_(matchAllMethod, nullptr)
{
	for (int i = 0; i < 4; i++)
	{
		indexMasks_[i].keyMask = keyMask;
		indexMasks_[i].keyMin = keyMin;
	}
}

void MatcherHolder::dealloc() const
{
	const uint8_t* p = reinterpret_cast<const uint8_t*>(this) - resourcesLength_;

	// deref any foreign matchers
	if (referencedMatcherHoldersCount_)
	{
		const MatcherHolder* const * ppChildMatcher =
			reinterpret_cast<const MatcherHolder* const *>(p);
		const MatcherHolder* const * ppEndChildMatcher = 
			ppChildMatcher + referencedMatcherHoldersCount_;
			reinterpret_cast<const MatcherHolder*>(p);
		while (ppChildMatcher < ppEndChildMatcher)
		{
			(*ppChildMatcher)->release();
			ppChildMatcher++;
		}
	}
	 
	// Destroy regex patterns
	if (regexCount_)
	{
		static_assert(alignof(std::regex) == 8, "std:regex must be 8-byte aligned");
		// (all resources are 8-byte aligned to accommodate natural alignment
		// of pointers, doubes and std::regex)
		const std::regex* pRegex = reinterpret_cast<const std::regex*>(
			p + sizeof(MatcherHolder*) * referencedMatcherHoldersCount_);
		const std::regex* pEndRegex = pRegex + regexCount_;
		while (pRegex < pEndRegex)
		{
			pRegex->std::regex::~regex();
			pRegex++;
		}
	}

	delete[] p;
}


const MatcherHolder* MatcherHolder::createMatchAll(FeatureTypes types)
{
	return new MatcherHolder(types);
}

/**
 * A matcher that checks for [k=v], where k and v are both global strings.
 */
class GlobalTagMatcher : public Matcher
{
public:
    GlobalTagMatcher(int keyCode, int valueCode) :
        Matcher(matchKeyValue, nullptr),	// don't need store access
        tagBits_((keyCode << 2) | (valueCode << 16) | 1) {}

	static bool matchKeyValue(const Matcher* matcher, const uint8_t* pFeature)
	{
		uint32_t tagBits = ((GlobalTagMatcher*)matcher)->tagBits_;
		uint16_t keyBits = static_cast<uint16_t>(tagBits);
		pointer p(pFeature+8);
		p = p.followTagged(~1);
		for (; ; )
		{
			// TODO: maybe just fetch key/value separately
			// to avoid unaligned read issue altogether
			uint32_t tag = p.getUnalignedUnsignedInt();
			if ((tag & 0xffff) >= keyBits)
			{
				return (tag & 0xffff'7fff) == tagBits;
			}
			p += 4 + (tag & 2);
		}
	}

private:
    uint32_t tagBits_;
};

/**
 * A matcher that checks for [k], where k is a global string (k must be
 * present and must not be "no")
 */
class GlobalKeyMatcher : public Matcher
{
public:
	GlobalKeyMatcher(int keyCode, int codeNo) :
		Matcher(matchKeyValue, nullptr),	// don't need store access
		tagBits_((keyCode << 2) | (codeNo << 16)) {}

	static bool matchKeyValue(const Matcher* matcher, const uint8_t* pFeature)
	{
		uint32_t tagBits = ((GlobalKeyMatcher*)matcher)->tagBits_;
		uint16_t keyBits = static_cast<uint16_t>(tagBits);
		pointer p(pFeature + 8);
		p = p.followTagged(~1);
		for (; ; )
		{
			uint32_t key = p.getUnsignedShort();
			if (key > keyBits)
			{
				uint32_t tag = (p.getUnsignedShort(2) << 16) | (key & 3);
				return (key & 0x7ffc) == keyBits && tag != 
					((tagBits & 0xffff'0000) | 1);
				// key matches, but it's value is not global string "no"
			}
			p += 4 + (key & 2);
		}
	}

private:
	uint32_t tagBits_;
};


// TODO: doesn't work, need mask, check "no"
const MatcherHolder* MatcherHolder::createMatchKey(
	FeatureTypes types, uint32_t indexBits, int keyCode, int codeNo)
{
	size_t size = sizeof(MatcherHolder) + sizeof(GlobalKeyMatcher) - sizeof(Matcher);
	MatcherHolder* self = (MatcherHolder*)alloc(size);

	// Since there is only one tag, mask and min are the same
	new (self) MatcherHolder(types, indexBits, indexBits);
	new (&self->mainMatcher_)GlobalKeyMatcher(keyCode, codeNo);
	return self;
}


const MatcherHolder* MatcherHolder::createMatchKeyValue(
	FeatureTypes types, uint32_t indexBits, int keyCode, int valueCode)
{
	size_t size = sizeof(MatcherHolder) + sizeof(GlobalTagMatcher) - sizeof(Matcher);
	MatcherHolder* self = (MatcherHolder*)alloc(size);

	// Since there is only one tag, mask and min are the same
	new (self) MatcherHolder(types, indexBits, indexBits);
	new (&self->mainMatcher_)GlobalTagMatcher(keyCode, valueCode);
	return self;
}


class ComboMatcher : public Matcher
{
public:
	ComboMatcher(FeatureStore* store) :
		Matcher(matchCombo, store) {}

	static bool matchCombo(const Matcher* matcher, const uint8_t* pFeature)
	{
		const uint8_t* p = reinterpret_cast<const uint8_t*>(matcher) -
			offsetof(MatcherHolder, mainMatcher_) - sizeof(MatcherHolder*) * 2;
		const MatcherHolder* const* pChildMatcher =
			reinterpret_cast<const MatcherHolder* const*>(p);
		return (*pChildMatcher)->mainMatcher_.accept(pFeature) &&
			(*(pChildMatcher + 1))->mainMatcher_.accept(pFeature);
	}
};


const MatcherHolder* MatcherHolder::combine(
	const MatcherHolder* a, const MatcherHolder* b)
{
	// A MatcherHolder that combines the results of two matchers needs
	// to store the pointers to the two child matchers at the beginning
	// of its resource section
	size_t resourceSize = sizeof(MatcherHolder*) * 2;
	uint8_t* matcherData = new uint8_t[sizeof(MatcherHolder) + resourceSize];
	MatcherHolder* self = reinterpret_cast<MatcherHolder*>(matcherData + resourceSize);

	// TODO: Currently, we use the same index bits for all 4 types
	// This may change in the future
	// TODO: What happens if one matcher can match *any* key, but
	//  the other matcher must match *all* keys?
	// To be safe, we use the *smaller* keyMin (The requirement to match
	// multiple indexed keys is very rare, especially when combining with
	// another matcher)
	// No: We use the larger keyMin in any case -- TODO: check this
	// TODO: Also see comment in MatcherCompiler::compileMatcher(),
	// currently we are always using keyMin 1 
	uint32_t keyMask = a->indexMasks_[0].keyMask | b->indexMasks_[0].keyMask;
	uint32_t keyMin = std::max(a->indexMasks_[0].keyMin, b->indexMasks_[0].keyMin);

	new (self)MatcherHolder(
		a->acceptedTypes() & b->acceptedTypes(), 
		keyMask, keyMin);
	new (&self->mainMatcher_)ComboMatcher(a->mainMatcher_.store());
	self->resourcesLength_ = resourceSize;
	self->referencedMatcherHoldersCount_ = 2;
	const MatcherHolder** pChildMatcher = 
		reinterpret_cast<const MatcherHolder**>(matcherData);
	a->addref();
	*pChildMatcher = a;
	b->addref();
	*(pChildMatcher+1) = b;
	return self;
}

