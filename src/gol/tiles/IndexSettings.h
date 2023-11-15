#pragma once

#include <cassert>
#include <cstdint>
#include <unordered_map> 

class FeatureStore;

class IndexSettings 
{
    IndexSettings(FeatureStore* store, int rtreeBucketSize, 
        int maxKeyIndexes, int keyIndexMinFeatures) :
        rtreeBucketSize_(rtreeBucketSize),
        maxKeyIndexes_(maxKeyIndexes),
        keyIndexMinFeatures_(keyIndexMinFeatures_),
        maxIndexedKey_(0) // TODO
    {
        // TODO        
    }

private:
    const int rtreeBucketSize_;
    const int maxKeyIndexes_;
    const int keyIndexMinFeatures_;
    const int maxIndexedKey_;
    const std::unordered_map<int, int> keysToCategory;
};
