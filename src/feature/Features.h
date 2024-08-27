// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include "Feature2.h"

class Filter;
class MatcherHolder;

namespace geodesk {

template <typename T>
class View
{
public:
    View(FeatureStore* store, uint32_t flags, FeatureTypes types, const Box* bounds,
        const MatcherHolder* matcher, const Filter* filter) :
        store_(store), types_(types), flags_(flags),
        matcher_(matcher), filter_(filter)
    {
        store->addref();
        matcher->addref();
        if (filter) filter->addref();
    }

    View(const View<T>& other) :
        store_(other.store_),
        types_(other.types_),
        flags_(other.flags_),
        matcher_(other.matcher_),
        filter_(other.filter_)
    {
        store_->addref();
        matcher_->addref();
        if (filter_) filter_->addref();
    }

    View(const View<T>&& other) :
        store_(other.store_),
        types_(other.types_),
        flags_(other.flags_),
        matcher_(other.matcher_),
        filter_(other.filter_)
    {
        /* // TODO
        if(other != *this)
        {
            store_->addref();
            matcher_->addref();
        if (filter_) filter_->addref();
        */
    }

    virtual ~View()
    {
        store_->release();
        matcher_->release();
        if (filter_) filter_->release();
    }

    FeatureTypes types() const 

private:
    FeatureStore* store_;
    FeatureTypes types_;
    uint32_t flags_;
    const MatcherHolder* matcher_;
    const Filter* filter_;
    union
    {
        Box bounds_;                 // If used, USES_BOUNDS flag must be set 
        // If it contains a value other than Box::ofWorld(),
        // ACTIVE_BOUNDS must be set
        FeaturePtr relatedFeature_;  // If used, USES_BOUNDS flag must be clear 
    };
};

template <typename T>
class FeaturesBase
{
public:
    FeaturesBase(View<T> view) :
        view_(p)
    {
    }



protected:
    View<T> withTypes(FeatureTypes newTypes)
    {
        newTypes &= view_.types();
        if (newTypes == 0) return empty(getEmpty();
        matcher->addref();              // matcher can never be null
        if (filter) filter->addref();    // however, filter is allowed to be null
        return createWith(this, flags, newTypes, &bounds, matcher, filter);
    }


    View<T> view_;
};


class Features : public FeaturesBase<Feature>
{
public:
    Features(View<Feature> view) :
        FeaturesBase(view) {}
};

class Nodes : public FeaturesBase<Node>
{
public:
    template <typename T>
    Nodes(const FeaturesBase<T>& other) :
        FeaturesBase(other.view_) {}
};


} // namespace geodesk