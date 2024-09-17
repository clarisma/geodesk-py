// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include "Feature.h"
#include <geodesk/filter/Filters.h>
#include "Query.h"
#include "QueryException.h"
#include "View.h"
#include "filter/PredicateFilter.h"

class Filter;
class MatcherHolder;
class PreparedFilterFactory;

// forward-declare the derived classes, so we can declare them as friends
// (otherwise, the derived classes can only access the base class of the 
// same specialization)
namespace geodesk
{
class Features;
class Nodes;
class Ways;
class Relations;
class Way;
class Relation;
}

namespace geodesk::detail {

using geodesk::Feature;

// TODO: type restrictions on derived

template <typename T>
class FeaturesBase
{
public:
    FeaturesBase(const FeaturesBase<T>& other) :
        view_(other.view_)
    {
    }

    FeaturesBase& operator=(const FeaturesBase& other)
    {
        view_ = other.view_;
    }

    operator bool() const { return !query().isEmpty(); }
    bool operator !() const { return query().isEmpty(); }

    template<typename T2>
    FeaturesBase operator&(const FeaturesBase<T2>& other) const;

    
    /// @name Query by type & tags
    /// @{

    FeaturesBase operator()(const char* query) const
    {
        return FeaturesBase(view_.withQuery(query));
    }

    FeaturesBase operator()(const Feature& feature) const
    {
        return intersecting(feature);
    }

    /// @brief Returns all features whose bounding box intersects
    /// the given bounding box.
    ///
    /// @param box
    FeaturesBase operator()(const Box& box) const
    {
        return { view_.withBounds(box) };
    }

    /// @brief Returns all features whose bounding box contains
    /// the given Coordinate.
    ///
    /// @param xy
    FeaturesBase operator()(Coordinate xy) const
    {
        return { view_.withBounds(xy) };
    }
    
    /// @}
    /// @name Single-feature queries
    /// @{

    // TODO :may be a bad idea, even if explicit
    //  User may have a typo -- "Feature" instead of "Features"
    //    Feature world = Features("world")
    //  This creates a Feature, assigning it the first
    //  random feature in world.gol; worse, the lifetime
    //  of the Features object is temporary, so its
    //  FeatureStore is immediately closed and `world`
    //  cannot even be legally accessed.
    //  Better to give users just the explicit options of
    //  `one()` and `first()`, which makes it clear what
    //  they're getting
    explicit operator T() const
    {
        Query<T> query(view_);
        if(query != nullptr) return *query;
        throw QueryException("No feature found");
    }

    std::optional<T> first() const
    {
        Query<T> query(view_);
        if(query != nullptr) return std::optional<T>(*query);
        return std::nullopt;
    }

    T one() const
    {
        Query<T> query(view_);
        if(query != nullptr)
        {
            T feature = *query;
            ++query;
            if(query != nullptr)
            {
                throw QueryException("More than one feature found");
            }
            return feature;
        }
        throw QueryException("No feature found");
    }

    operator std::vector<T>() const;

    /// @}
    /// @name Scalar queries
    /// @{

    /// Returns the number of features in this collection.
    ///
    uint64_t count() const
    {
        return query().count();
    }

    Query<T> begin() const
    {
        return Query<T>(view_);
    }
    std::nullptr_t end() const
    {
        return nullptr;  // Simple sentinel indicating the end of iteration
    }

    /// @}
    /// @name Spatial filters
    /// @{

    /// @brief Returns all features whose geometry intersects with the
    /// given Feature.
    ///
    FeaturesBase intersecting(const Feature& feature) const
    {
        return { view_.withFilter(Filters::intersects(feature))};
    }

    /// @brief Returns all features that lie entirely inside the geometry
    /// of the given Feature.
    ///
    FeaturesBase within(const Feature& feature) const
    {
        return {view_.withFilter(Filters::within(feature))};
    }

    /// @}
    /// @name Topological filters
    /// @{

    template<typename Ptr, bool N, bool W, bool R>
    FeaturesBase membersOf(FeatureBase<Ptr,N,W,R> feature) const;

    /// @}

    template <typename Predicate>
    FeaturesBase filter(Predicate predicate) const
    {
        return FeaturesBase(view_.withFilter(
            new PredicateFilter<Predicate>(predicate)));
    }

protected:
    FeaturesBase(const View& view) : view_(view)
    {
    }

    View empty() const
    {
        return view_.empty();
    }

    Query<T> query() const
    {
        return Query<T>(view_);
    }

    bool isEmpty() const
    {
        return Query<T>(view_).isEmpty();
    }


    static View rootView(const char* golFile)
    {
        FeatureStore* store = FeatureStore::openSingle(golFile);
        const MatcherHolder* matcher = store->getAllMatcher();
        return View(View::WORLD, 0, FeatureTypes::ALL, store,
            Box::ofWorld(), matcher, nullptr);
    }

    FeaturesBase withFilter(PreparedFilterFactory& factory, Feature feature) const;

    View view_;

    friend class geodesk::Features;
    friend class geodesk::Nodes;
    friend class geodesk::Ways;
    friend class geodesk::Relations;
};

} // namespace geodesk::detail