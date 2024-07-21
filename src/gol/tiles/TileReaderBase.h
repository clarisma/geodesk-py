// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include <cstdint>
#include <common/util/DataPtr.h>
#include "feature/Node.h"
#include "feature/Way.h"
#include "feature/Relation.h"
#include "TileConstants.h"

using namespace TileConstants;

template<typename Derived>
class TileReaderBase
{
public:
	void readTileFeatures(const uint8_t* pTile)
	{
		readNodes(pTile + NODE_INDEX_OFS);
		readFeatures(pTile + WAY_INDEX_OFS);
		readFeatures(pTile + AREA_INDEX_OFS);
		readFeatures(pTile + RELATION_INDEX_OFS);
	}

    void readNodes(DataPtr ppIndex)
    {
        int32_t rel = ppIndex.getInt();
        if (rel == 0) return;
        if ((rel & 1) == 0)
        {
            readNodeRoot(ppIndex);
            return;
        }
        DataPtr p = ppIndex + (rel ^ 1);
        for (;;)
        {
            int last = p.getInt() & 1;
            readNodeRoot(p);
            if (last != 0) break;
            p += 8;
        }
    }

    void readNodeRoot(DataPtr ppTree)
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

    void readRoot(DataPtr ppTree)
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

    void readNodeTree(DataPtr p)
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

    void readNodeLeaf(DataPtr p)
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

    void readFeatures(DataPtr ppTree)
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

    void readTree(DataPtr p)
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

    void readLeaf(DataPtr p)
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
};
