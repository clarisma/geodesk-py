// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include "feature/TagTablePtr.h"

class AbstractTagIterator
{
public:
    AbstractTagIterator(int_fast32_t handle, TagTablePtr tags) :
        pTile_(tags.ptr() - handle),
        ofs_(handle),
        keyBits_(0)
    {
    }

    AbstractTagIterator(const uint8_t* pTile, TagTablePtr tags) :
        pTile_(pTile),
        ofs_(DataPtr::nearDelta(tags.ptr() - pTile_)),
        keyBits_(0)
    {
    }

    uint32_t keyBits() const { return keyBits_; }
    bool hasStringValue() const { return keyBits_ & 1; }
    bool hasWideValue() const { return keyBits_ & 2; }
    bool hasLocalStringValue() const { return (keyBits_ & 3) == 3; }
    DataPtr ptr() const { return pTile_ + ofs_; }

protected:
    AbstractTagIterator(const uint8_t* pTile, int_fast32_t ofs) :
        pTile_(pTile),
        ofs_(ofs),
        keyBits_(0)
    {
    }

    DataPtr pTile_;
    int_fast32_t ofs_;
    int32_t keyBits_;			// must be signed for locals
};


