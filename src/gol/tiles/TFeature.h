#pragma once

#include "TElement.h"
#include <common/util/pointer.h>
#include "feature/Node.h"
#include "feature/Way.h"
#include "feature/Relation.h"

class TFeature : public TIndexedElement
{
public:
	TFeature(int32_t loc, uint32_t size) :
		TIndexedElement(loc, aligned4(size))
	{
	}
};

class TNode : public TFeature
{
public:
	TNode(int32_t loc, NodeRef node) :
		TFeature(loc, 20 + (node.flags() & 4)),		// Bit 2 = member flag
		node_(node)
	{
	}

private:
	NodeRef node_;
};

class TWayBody : public TElement
{
public:
	TWayBody(pointer data, SizeAndAlignment sizeAndAlignment, uint32_t anchor) :
		TElement(-1, sizeAndAlignment),
		data_(data),
		anchor_(anchor)
	{
	}

private:
	pointer data_;
	uint32_t anchor_;
};

class TWay : public TFeature
{
public:
	TWay(int32_t loc, WayRef way, pointer pBodyData, SizeAndAlignment bodySize, uint32_t bodyAnchor) :
		TFeature(loc, 32),
		way_(way),
		body_(pBodyData, bodySize, bodyAnchor)
	{
	}

private:
	WayRef way_;
	TWayBody body_;
};


