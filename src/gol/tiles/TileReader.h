// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include <cstdint>
#include <common/util/pointer.h>
#include "feature/Node.h"
#include "feature/Way.h"
#include "feature/Relation.h"

template<typename Derived>
class TileReader
{
public:
	void readTileFeatures(const uint8_t* pTile)
	{
		readNodes(pTile + NODE_INDEX_OFS);
		readFeatures(pTile + WAY_INDEX_OFS);
		readFeatures(pTile + AREA_INDEX_OFS);
		readFeatures(pTile + RELATION_INDEX_OFS);
	}

    void readNodes(pointer ppIndex)
    {
        int32_t rel = ppIndex.getInt();
        if (rel == 0) return;
        if ((rel & 1) == 0)
        {
            readNodeRoot(ppIndex);
            return;
        }
        pointer p = ppIndex + (rel ^ 1);
        for (;;)
        {
            int last = p.getInt() & 1;
            readNodeRoot(p);
            if (last != 0) break;
            p += 8;
        }
    }

    void readNodeRoot(pointer ppTree)
    {
        int32_t rel = ppTree.getInt();
        if (rel != 0)
        {
            if ((rel & 2) != 0)
            {
                readNodeLeaf(ppTree + (rel & 0xffff'fffc));
            }
            else
            {
                readNodeTree(ppTree + (rel & 0xffff'fffc));
            }
        }
    }

    void readRoot(pointer ppTree)
    {
        int32_t rel = ppTree.getInt();
        if (rel != 0)
        {
            if ((rel & 2) != 0)
            {
                readLeaf(ppTree + (rel & 0xffff'fffc));
            }
            else
            {
                readTree(ppTree + (rel & 0xffff'fffc));
            }
        }
    }

    void readNodeTree(pointer p)
    {
        for (;;)
        {
            int32_t rel = p.getInt();
            int last = rel & 1;
            if ((rel & 2) != 0)
            {
                readNodeLeaf(p + (rel ^ 2 ^ last));
            }
            else
            {
                readNodeTree(p + (rel ^ last));
            }
            if (last != 0) break;
            p += 20;
        }
    }

    void readNodeLeaf(pointer p)
    {
        p += 8;
        for (;;)
        {
            int flags = p.getInt();
            self().readNode(NodeRef(p));
            if ((flags & 1) != 0) break;
            p += 20 + (flags & 4);
            // If Node is member of relation (flag bit 2), add
            // extra 4 bytes for the relation table pointer
        }
    }

    void readFeatures(pointer ppTree)
    {
        int32_t rel = ppTree.getInt();
        if (rel == 0) return;
        if ((rel & 1) == 0)
        {
            readRoot(ppTree);
            return;
        }
        pointer p = ppTree + (rel ^ 1);
        for (;;)
        {
            int last = p.getInt() & 1;
            readRoot(p);
            if (last != 0) break;
            p += 8;
        }
    }

    void readTree(pointer p)
    {
        for (;;)
        {
            int32_t rel = p.getInt();
            int last = rel & 1;
            if ((rel & 2) != 0)
            {
                readLeaf(p + (rel ^ 2 ^ last));
            }
            else
            {
                readTree(p + (rel ^ last));
            }
            if (last != 0) break;
            p += 20;
        }
    }

    void readLeaf(pointer p)
    {
        p += 16;
        for (;;)
        {
            int flags = p.getInt();
            if ((flags & (3 << 3)) == (1 << 3))
            {
                self().readWay(WayRef(p));
            }
            else
            {
                assert((flags & (3 << 3)) == (2 << 3));
                self().readRelation(RelationRef(p));
            }
            if ((flags & 1) != 0) break;
            p += 32;
        }
    }

private:
    Derived& self() 
    {
        return *static_cast<Derived*>(this);
    }

	static const int NODE_INDEX_OFS = 8;
	static const int WAY_INDEX_OFS = 12;
	static const int AREA_INDEX_OFS = 16;
	static const int RELATION_INDEX_OFS = 24;
};
