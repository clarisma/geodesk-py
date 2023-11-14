#pragma once

#include "TElement.h"
#include <common/util/pointer.h>

class TFeature;

class TIndexLeaf : public TElement
{
public:
};

class TIndexTrunk : public TElement
{
public:
};


class TIndex : public TElement
{
public:

private:
	static const int MAX_CATEGORIES = 30;

	struct Root
	{
		int32_t indexBits;
		uint32_t count;
		TIndexTrunk* trunk;
		TFeature* firstFeature;
	};

	Root roots_[MAX_CATEGORIES + 2];
};
