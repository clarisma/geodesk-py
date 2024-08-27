// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include "FeatureStore.h"
#include "NodePtr.h"
#include "WayPtr.h"
#include "RelationPtr.h"

namespace geodesk {

template <typename T>
class FeatureBase
{
public:
    FeatureBase(FeatureStore* store, const T p, const ShortVarString* role) :
        p_(p),
        store_(store),
        role_(role)
    {
    }

protected:
    T safeCast(FeaturePtr p, int type)
    {
        if(p.typeCode() != type) 
        { 
            // TODO: throw
        }
        return T(p);
    }

    union
    {
        const TaggedPtr<const uint8_t,1> pTagged_;
        uint64_t taggedId_;
    };
    FeatureStore* store_;
    union
    {
        const ShortVarString* role_;
        Coordinate xy_;
    }
};

class Feature : public FeatureBase<FeaturePtr>
{
    Feature(FeatureStore* store, const FeaturePtr p, const ShortVarString* role) :
        FeatureBase(store, p, role) {}

    template <typename T>
    Feature(const FeatureBase<T>& other) :
        FeatureBase(other.store_, other.p_, other.role_) {}
};


class Node : public FeatureBase<NodePtr>
{
    Node(FeatureStore* store, const NodePtr p, const ShortVarString* role) :
        FeatureBase(store, p, role) {}
    
    template <typename T>
    Node(const FeatureBase<T>& other) :
        FeatureBase(other.store_, safeCast(other.p_, FeatureType::NODE), other.role_) {}
};


class Way : public FeatureBase<WayPtr>
{
    Way(FeatureStore* store, const WayPtr p, const ShortVarString* role) :
        FeatureBase(store, p, role) {}

    template <typename T>
    Way(const FeatureBase<T>& other) :
        FeatureBase(other.store_, safeCast(other.p_, FeatureType::WAY), other.role_) {}
};


class Relation : public FeatureBase<RelationPtr>
{
    Relation(FeatureStore* store, const RelationPtr p, const ShortVarString* role) :
        FeatureBase(store, p, role) {}

    template <typename T>
    Relation(const FeatureBase<T>& other) :
        FeatureBase(other.store_, safeCast(other.p_, FeatureType::RELATION), other.role_) {}
};


} // namespace geodesk
