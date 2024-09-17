// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include <filter/ComboFilter.h>

#include "QueryException.h"
#include "feature/FeatureStore.h"
#include "feature/types.h"
#include "geom/Box.h"

class Filter;
class MatcherHolder;

namespace geodesk::detail {

class View
{
    union Context
    {
        struct
        {
            int32_t minX, minY, maxX, maxY;
        }
        bounds;                 // If used, USES_BOUNDS flag must be set 
                                // If it contains a value other than Box::ofWorld(),
                                // ACTIVE_BOUNDS must be set
        const uint8_t* relatedFeature;  // If used, USES_BOUNDS flag must be clear 
    };

    enum Flags
    {
        /**
         * Context uses `bounds` (instead of `relatedFeature` in
         * the same union). Only used by WORLD selection.
         */
        USES_BOUNDS = 1,
        /**
         * `bounds` has bounds tighter than `Box::ofWorld`. `USES_BOUNDS` must be set.
         */
        BOUNDS_ACTIVE = 2,
        USES_MATCHER = 4,
        USES_FILTER = 8,        // TODO: is this used?

        // TODO: need flag to indicate if relatedFeature is in use
        // or does NOT USES_BOUNDS imply use of relatedFeature?
    };

public:
    enum
    {
        EMPTY,
        WORLD,
        WAY_NODES,
        MEMBERS,
        PARENTS,
        PARENT_WAYS,
        PARENT_RELATIONS,
    };

    // steals references
    View(int view, int flags, FeatureTypes types, FeatureStore* store, const Context& context,
        const MatcherHolder* matcher, const Filter* filter) :
        view_(view), flags_(flags), types_(types), store_(store), 
        context_(context), matcher_(matcher), filter_(filter)
    {
        //printf("Creating view, store refcount = %llu\n", store_->refcount());
        //fflush(stdout);
    }

    View(int view, int flags, FeatureTypes types, FeatureStore* store, const Box& bounds,
        const MatcherHolder* matcher, const Filter* filter) :
        view_(view), flags_(flags | USES_BOUNDS), types_(types), store_(store),
        matcher_(matcher), filter_(filter)
    {
        context_.bounds = { bounds.minX(), bounds.minY(), bounds.maxX(), bounds.maxY() };
    }

    View(int view, int flags, FeatureTypes types, FeatureStore* store, FeaturePtr related,
        const MatcherHolder* matcher, const Filter* filter) :
        view_(view), flags_(flags), types_(types), store_(store),
        matcher_(matcher), filter_(filter)
    {
        context_.relatedFeature = related.ptr();
    }


    View(const View& other) :
        View(other.view_, other.flags_, other.types_, other.store_,
            other.context_, other.matcher_, other.filter_)
    {
        // printf("Creating copy of view, store refcount before = %llu\n", store_->refcount());
        // fflush(stdout);
        // caller must inc refcounts, but here, this method is
        // the caller
        store_->addref();
        matcher_->addref();
        if (filter_) filter_->addref();
        //printf("  store refcount after = %llu\n", store_->refcount());
        //fflush(stdout);
    }

    ~View()
    {
        //printf("Destroying view, store refcount before = %llu\n", store_->refcount());
        //fflush(stdout);
        store_->release();
        matcher_->release();
        if (filter_) filter_->release();
    }

    View& operator=(const View& other)
    {
        //printf("Assigning view (store refcount before = %llu)\n", store_->refcount());
        //fflush(stdout);
        view_ = other.view_;
        flags_ = other.flags_;
        types_ = other.types_;
        if(store_ != other.store_)
        {
            store_->release();
            other.store_->addref();
            store_ = other.store_;
        }
        context_ = other.context_;
        if(matcher_ != other.matcher_)
        {
            matcher_->release();
            other.matcher_->addref();
            matcher_ = other.matcher_;
        }
        if(filter_ != other.filter_)
        {
            if(filter_) filter_->release();
            if(other.filter_) other.filter_->addref();
            filter_ = other.filter_;
        }
        // printf("  Assigned. (store refcount now = %llu)\n", store_->refcount());
        // fflush(stdout);
        return *this; // Return the current object
    }


    uint32_t view() const { return view_; }
    FeatureStore* store() const { return store_; }
    FeatureTypes types() const { return types_; }
    const MatcherHolder* matcher() const { return matcher_; }
    const Filter* filter() const { return filter_; }
    Box bounds() const 
    { 
        return Box(context_.bounds.minX, context_.bounds.minY, 
            context_.bounds.maxX, context_.bounds.maxY);
    }

    /* // TODO: bad?
    View& operator=(const View& other)
    {
        view_ = other.view_;
        flags_ = other.flags_;
        types_ = other.types_;
        other.store_->addref();
        other.matcher_->addref();
        if (other.filter_) other.filter_->addref();
        store_->release();
        matcher_->release();
        if (filter_) filter_->release();
        store_ = other.store_;
        context_ = other.context_;
        matcher_ = other.matcher_;
        filter_ = other.filter_;
        return *this;
    }
    */

    View operator&(FeatureTypes types) const
    {
        types &= types_;
        if (types == 0) return empty();
        store_->addref();
        matcher_->addref();
        if (filter_) filter_->addref();
        return View(view_, flags_, types, store_, context_, matcher_, filter_);
    }

    View withQuery(const char* query, FeatureTypes newTypes = FeatureTypes::ALL) const
    {
        // TODO: Turn ParseException into QueryException
        //try
        //{
            const MatcherHolder* newMatcher = store_->getMatcher(query);
            newTypes &= types_ & newMatcher->acceptedTypes();
            if (newTypes == 0)
            {
                newMatcher->release();
                return empty();
            }
            if (flags_ & USES_MATCHER)
            {
                matcher_->addref();
                newMatcher = MatcherHolder::combine(matcher_, newMatcher);
                // combine() steals references
            }

            if (filter_) filter_->addref();     
            store_->addref();
            return View(view_, flags_ | USES_MATCHER, newTypes, store_,
                context_, newMatcher, filter_);
        // }
        /*
        catch (const ParseException& ex)
        {
            // Environment::get().raiseQueryException("Bad query: %s", ex.what());
            Environment::get().raiseQueryException(ex.what());
            return NULL;
        }
        */
    }

    View withFilter(const Filter* newFilter) const
    {
        if (filter_)
        {
            const ComboFilter* combo = new ComboFilter(filter_, newFilter);
            newFilter->release();
            // Need to release because this function is expected to consume the
            // reference (ComboFilter adds its own ref)
            newFilter = combo;
        }
        FeatureTypes newTypes = types_ & newFilter->acceptedTypes();
        if (newTypes == 0)
        {
            newFilter->release();
            return empty();
        }

        // TODO: apply bounds!!!
        //
        // TODO: fix
        Box b = newFilter->getBounds();
        // Box b = Box::simpleIntersection(bounds, newFilter->getBounds());

        matcher_->addref();      // createWith consumes ref to matcher
        store_->addref();
        if(flags_ & USES_BOUNDS)
        {
            Context ctx;
            ctx.bounds = { b.minX(), b.minY(), b.maxX(), b.maxY() };
            return View(view_, flags_ | USES_FILTER, newTypes, store_,
                ctx, matcher_, newFilter);
        }
        return View(view_, flags_ | USES_FILTER, newTypes, store_,
            context_, matcher_, newFilter);
    }

    View withBounds(Box box) const
    {
        if (box.isEmpty()) return empty();
        if (flags_ & USES_BOUNDS)
        {
            if (flags_ & BOUNDS_ACTIVE)
            {
                box = Box::simpleIntersection(box, bounds());
                if(box.isEmpty()) return empty();
            }
            matcher_->addref();
            if(filter_) filter_->addref();
            store_->addref();
            return View(view_, flags_ | BOUNDS_ACTIVE, types_, store_, box, matcher_, filter_);
        }
        throw QueryException("Not yet implented");
    }

    View withBounds(Coordinate xy) const
    {
        if (flags_ & USES_BOUNDS)
        {
            if (flags_ & BOUNDS_ACTIVE)
            {
                if(!bounds().contains(xy)) return empty();
            }
            matcher_->addref();
            if(filter_) filter_->addref();
            store_->addref();
            return View(view_, flags_ | BOUNDS_ACTIVE, types_, store_, Box(xy), matcher_, filter_);
        }
        throw QueryException("Not yet implented");
    }

    View ofRelated(int view, FeaturePtr related, FeatureTypes types = FeatureTypes::ALL) const
    {
        types &= types_;
        if(types == 0) return empty();
        // TODO: If bbox is active, wrap it into a filter
        matcher_->addref();
        if(filter_) filter_->addref();
        store_->addref();
        return View(view, flags_ & ~(USES_BOUNDS | BOUNDS_ACTIVE), types, store_,
            related, matcher_, filter_);
    }

    View parentRelationsOf(FeaturePtr related) const
    {
        return ofRelated(PARENT_RELATIONS, related);
    }

    View membersOfRelation(FeaturePtr related) const
    {
        assert(related.isRelation());
        return ofRelated(MEMBERS, related, FeatureTypes::RELATION_MEMBERS);
    }


    View empty() const
    {
        store_->addref();
        matcher_->addref();
        return View(EMPTY, 0, 0, store_, FeaturePtr(), matcher_, nullptr);
    }

private:
    uint16_t view_;
    uint16_t flags_;
    FeatureTypes types_;
    FeatureStore* store_;
    const MatcherHolder* matcher_;
    const Filter* filter_;
    Context context_;
};

} // namespace geodesk::detail
