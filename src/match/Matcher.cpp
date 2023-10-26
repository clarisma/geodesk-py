// Copyright (c) 2023 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#include "Matcher.h"

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

	// TODO: deref any foreign matchers
	// TODO: destroy regex patterns

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

