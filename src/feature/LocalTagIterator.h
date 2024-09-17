// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include "feature/AbstractTagIterator.h"

class LocalTagIterator : public AbstractTagIterator
{
public:
    LocalTagIterator(int_fast32_t handle, TagTablePtr tags) :
        AbstractTagIterator(tags.ptr() - handle, handle),
        originOfs_(handle & 0xffff'fffc)
    {
        keyBits_ = tags.hasLocalKeys() ? 0 : 4;
    }

    LocalTagIterator(const uint8_t* pTile, TagTablePtr tags) :
        AbstractTagIterator(pTile, DataPtr::nearDelta(tags.ptr() - pTile)),
        originOfs_((DataPtr::nearDelta(tags.ptr() - pTile)) & 0xffff'fffc)
    {
        keyBits_ = tags.hasLocalKeys() ? 4 : 0;
    }

    bool next()
    {
        if (keyBits_ & 4) return false;
        ofs_ -= 4;
        keyBits_ = (pTile_ + ofs_).getIntUnaligned();	// signed
        ofs_ -= 2 + (keyBits_ & 2);
        return true;
    }

    int32_t flags() const { return keyBits_ & 7; }

    int_fast32_t keyStringHandle() const
    {
        return originOfs_ + ((keyBits_ >> 1) & 0xffff'fffc);
    }

    DataPtr keyString() const
    {
        return pTile_ + keyStringHandle();
    }

    uint32_t value() const
    {
        // Cost of unaligned read is likely cheaper than branch misprediction
        return (pTile_ + ofs_).getUnsignedIntUnaligned() &
            ((keyBits_ & 2) ? 0xffff'ffff : 0xffff);
    }

    int_fast32_t stringValueHandleFast() const
    {
        assert(hasLocalStringValue());
        return ofs_ + (pTile_ + ofs_).getIntUnaligned();
    }

    DataPtr stringValueFast() const
    {
        assert(hasLocalStringValue());
        return pTile_ + stringValueHandleFast();
    }

protected:
    int_fast32_t originOfs_;
};

