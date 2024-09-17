// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include "feature/AbstractTagIterator.h"

class GlobalTagIterator : public AbstractTagIterator
{
public:
    using AbstractTagIterator::AbstractTagIterator;

    bool next()
    {
        if (keyBits_ & 0x8000) return false;
        keyBits_ = (pTile_ + ofs_).getUnsignedShort();

        // TODO: not needed in 2.0
        if (keyBits_ == 0xffff) keyBits_ = 0x8000;
        ofs_ += 4 + (keyBits_ & 2);
        return true;
    }

    uint32_t keyBits() const { return static_cast<uint32_t>(keyBits_); }
    uint32_t key() const { return (keyBits() >> 2) & 0x1fff; }
    uint32_t value() const
    {
        // TODO: Could always read an int, then mask off upper bits if narrow
        // Cost of unligned read vs. branch misprediction

        if (keyBits_ & 2)
        {
            return (pTile_ + ofs_ - 4).getUnsignedIntUnaligned();
        }
        else
        {
            return (pTile_ + ofs_ - 2).getUnsignedShort();
        }
    }

    int_fast32_t stringValueHandleFast() const
    {
        assert(hasLocalStringValue());
        int_fast32_t valueOfs = ofs_ - 4;
        return valueOfs + (pTile_ + valueOfs).getIntUnaligned();
    }

    DataPtr stringValueFast() const
    {
        assert(hasLocalStringValue());
        return pTile_ + stringValueHandleFast();
    }
};
