// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include "View.h"
#include "EmptyQuery.h"
#include "WorldQuery.h"

namespace geodesk {

using detail::View;
using detail::QueryAdapter;
// wrapper class around all the different queries

// TODO: careful, there's Query/AbstractQuery in query folder!

// TODO: AbstractQuery just needs to implement next()!
// no need to have virtual methods for ++ and != in the specializations

template<typename T>
class Query
{
public:
    Query(const View& view)
    {
        store_ = view.store();
        switch (view.view())
        {
        case View::EMPTY:
            new(&storage_.emptyQuery) detail::EmptyQuery(view);
            break;
        case View::WORLD:
            new (&storage_.worldQuery) detail::WorldQuery(view);
            break;
        }
        current_ = getBase()->next();
    }

    // Destructor: Manually invoke the destructor of the stored iterator
    ~Query()
    {
        getBase()->~QueryAdapter<AbstractQuery>();
    }

    // Dereference operator
    T operator*() const
    {
        return T(store_, current_);
    }

    // Increment operator
    Query& operator++()
    {
        current_ = getBase()->next();
        return *this;
    }

    // Comparison operator with nullptr
    bool operator!=(std::nullptr_t) const
    {
        return !current_.isNull();
    }

    bool isEmpty() const
    {
        return *getBase() == nullptr;
    }

    uint64_t count()
    {
        uint64_t count = 0;
        while(!current_.isNull())
        {
            current_ = getBase()->next();
            count++;
        }
        return count;
    }
	
private:
    QueryAdapter<AbstractQuery>* getBase()
    {
        // Return the base class pointer from the union
        return reinterpret_cast<QueryAdapter<AbstractQuery>*>(&storage_);
    }

    const QueryAdapter<AbstractQuery>* getBase() const
    {
        return reinterpret_cast<const QueryAdapter<AbstractQuery>*>(&storage_);
    }

    FeatureStore* store_;
	union Storage
	{
        detail::EmptyQuery emptyQuery;
		detail::WorldQuery worldQuery;

	    // Default constructor
	    Storage() {}
	    // Destructor does nothing as we handle destruction manually
	    ~Storage() {}
	}
	storage_;
    FeaturePtr current_;
};

} // namespace geodesk


// Possible iterators:
// - empty -> nothing
// - world view -> Node,Way,Relation
// - purely coordinate nodes -> AnonymousNode
// - purely feature nodes -> Node
// - mix of feature nodes and coordinate nodes -> Node, AnonymousNode
// - members -> Node,Way,Relation
// - parent relations -> Relation
// - parent ways (type of world view) -> Way
// - both parent relations & parent ways -> Way,Relation

