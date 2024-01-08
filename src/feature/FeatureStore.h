// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include <algorithm>
#include <unordered_map>
#include <common/store/BlobStore.h>
#include <common/util/Bits.h>
#include <common/util/ThreadPool.h>

#include "StringTable.h"
#include "match/Matcher.h"
#include "match/MatcherCompiler.h"
#include "query/TileQueryTask.h"

class MatcherHolder;

class ZoomLevels
{
public:
    inline int count()
    {
        return Bits::bitCount(m_levels);
    }

private:
    uint32_t m_levels;
};


//  Possible threadpool alternatives:
//  - https://github.com/progschj/ThreadPool (Zlib license, header-only)


class FeatureStore : public BlobStore
{
public:
    using IndexedKeyMap = std::unordered_map<uint16_t, uint16_t>;

    FeatureStore();
    ~FeatureStore();

    static FeatureStore* openSingle(std::string_view fileName);

    void open(const char* fileName)
    {
        BlobStore::open(fileName, 0);   // TODO: open mode
    }
    
    void addref()  { ++refcount_;  }
    void release() { if (--refcount_ == 0) delete this;  }

    pointer tileIndex() const 
    { 
        return getPointer(TILE_INDEX_PTR_OFS);
    }

    uint32_t zoomLevels() const { return zoomLevels_; }
    StringTable& strings() { return strings_; }
    const IndexedKeyMap& keysToCategories() const { return keysToCategories_; }
    int getIndexCategory(int keyCode) const;
    const MatcherHolder* getMatcher(const char* query);

    // TODO: This is duplicated in Environment
    const MatcherHolder* borrowAllMatcher() const { return &allMatcher_; }
    const MatcherHolder* getAllMatcher() 
    { 
        allMatcher_.addref();
        return &allMatcher_; 
    }
    bool isAllMatcher(const MatcherHolder* matcher) const
    {
        return matcher == &allMatcher_;
    }

    #ifdef GEODESK_PYTHON
    // PyObject* emptyString();
    PyObject* emptyTags();
    #endif

    ThreadPool<TileQueryTask>& executor() { return executor_; }

    pointer fetchTile(Tip tip);

protected:
    void initialize() override;

    pointer getPointer(int ofs) const
    {
        return pointer(mainMapping()).follow(ofs);
    }

private:
	static const uint32_t SubtypeMagic = 0x1CE50D6E;

    static const uint32_t ZOOM_LEVELS_OFS = 40;
    static const uint32_t TILE_INDEX_PTR_OFS = 44;
    static const uint32_t STRING_TABLE_PTR_OFS = 52;
    static const uint32_t INDEX_SCHEMA_PTR_OFS = 56;

    void readIndexSchema();
    
    size_t refcount_;
    StringTable strings_;
    IndexedKeyMap keysToCategories_;
    MatcherCompiler matchers_;
    MatcherHolder allMatcher_;
    #ifdef GEODESK_PYTHON
    PyObject* emptyFeatures_;       
        // Not ideal, should have global singleton instead of per-store,
        // but PyFeatures requires a non-null MatcherHolder, which in turn
        // requires a FeatureStore
    #endif
    ThreadPool<TileQueryTask> executor_;
    uint32_t zoomLevels_;

    static std::unordered_map<std::string, FeatureStore*> openStores_;
};

