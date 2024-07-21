// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include "TElement.h"

/**
 * A TElement that can be indexed by its Handle 
 * (TFeature, TString, TTagTable, TRelationTable)
 */
class TReferencedElement : public TElement
{
public:
	TReferencedElement(Type type, Handle handle, uint32_t size,
		Alignment alignment, int anchor) :
		TElement(type, handle, size, alignment, anchor),
		nextByHandle_(nullptr)
	{
	}

private:
	TReferencedElement* nextByHandle_;

	friend class LookupByHandle;
};


class LookupByHandle : public Lookup<LookupByHandle, TReferencedElement>
{
public:
	static uint64_t getId(TReferencedElement* element)
	{
		return element->handle();
	}

	static TReferencedElement** next(TReferencedElement* elem)
	{
		return &elem->nextByHandle_;
	}
};

