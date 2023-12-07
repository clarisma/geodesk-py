#pragma once

#include "feature/FeatureStore.h"

class IndexSettings 
{
public:
    IndexSettings(FeatureStore* store, int rtreeBucketSize, 
        int maxKeyIndexes, int keyIndexMinFeatures) :
        rtreeBucketSize_(rtreeBucketSize),
        maxKeyIndexes_(maxKeyIndexes),
        keyIndexMinFeatures_(keyIndexMinFeatures),
        keysToCategories_(store->keysToCategories()),
        maxIndexedKey_(findMaxIndexedKey(store->keysToCategories()))
    {
    }

    int rtreeBucketSize() const { return rtreeBucketSize_; }
    int maxKeyIndexes() const { return maxKeyIndexes_; }
    int keyIndexMinFeatures() const { return keyIndexMinFeatures_; }
    int maxIndexedKey() const { return maxIndexedKey_; }
    int getCategory(int key) const
    {
        auto it = keysToCategories_.find(key);
        return (it != keysToCategories_.end()) ? it->second : 0;
    }

private:
    static int findMaxIndexedKey(const FeatureStore::IndexedKeyMap& map)
    {
        int maxKey = 0;
        for (const auto& pair : map)
        {
            if (pair.first > maxKey) maxKey = pair.first;
        }
        return maxKey;
    }

    const int rtreeBucketSize_;
    const int maxKeyIndexes_;
    const int keyIndexMinFeatures_;
    const int maxIndexedKey_;
    FeatureStore::IndexedKeyMap keysToCategories_;
};

