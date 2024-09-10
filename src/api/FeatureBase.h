// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include <common/text/Format.h>
#include <common/util/Strings.h>
#include <common/util/TaggedPtr.h>
#include "feature/NodePtr.h"
#include "feature/WayPtr.h"
#include "feature/RelationPtr.h"
#include "feature/FeatureStore.h"
#include "geom/Box.h"
#include "geom/Centroid.h"
#include "geom/Mercator.h"
#include "Tags.h"

// TODO: Idea: move all logic into the abstract classes, then make aliases instead of classes
//  Then create "fake" Node, Way, etc. to feed into Doxygen

namespace geodesk
{
class Features;
class Nodes;
}

namespace geodesk::detail {

/// \internal
///
/// Do not show this class.
///
/// @tparam Ptr
/// @tparam N
/// @tparam W
/// @tparam R
template<typename Ptr, bool N, bool W, bool R>
class FeatureBase
{
    using Feature_ = FeatureBase<FeaturePtr,true,true,true>;
    using Node_ = FeatureBase<NodePtr,true,false,false>;

public:
    FeatureBase(FeatureStore* store, FeaturePtr ptr, const ShortVarString* role = nullptr)
    {
        store_ = store;
        feature_.ptr = Ptr(ptr);
        feature_.role = role;
    }
    FeatureBase(const FeatureBase<FeaturePtr,true, true, true>& other)     // Feature
    {
        if (!(N && W && R))
        {
            if (N && !other.isNode()) wrongType(other);
            if (W && !other.isWay()) wrongType(other);
            if (R && !other.isRelation()) wrongType(other);
        }
        copyFrom(other);
    }

    FeatureBase(const FeatureBase<NodePtr, true, false, false>& node)   // Node
    {
        if (!N) wrongType(node);
        copyFrom(node);
    }

    FeatureBase(const FeatureBase<WayPtr, false, true, false>& way)   // Way
    {
        if (!W) wrongType(way);
        copyFrom(way);
    }

    FeatureBase(const FeatureBase<RelationPtr, false, false, true>& relation)   // Relation
    {
        if (!R) wrongType(relation);
        copyFrom(relation);
    }

    FeatureBase& operator=(const FeatureBase<FeaturePtr,true, true, true>& other) // = Feature
    {
        if (!(N && W && R))
        {
            if (N && !other.isNode()) wrongType(other);
            if (W && !other.isWay()) wrongType(other);
            if (R && !other.isRelation()) wrongType(other);
        }
        copyFrom(other);
        return *this;
    }

    FeatureBase& operator=(const FeatureBase<NodePtr,true,false,false>& node) // = Node
    {
        if (!N) wrongType(node);
        copyFrom(node);
        return *this;
    }

    FeatureBase& operator=(const FeatureBase<WayPtr,false, true, false>& way)
    {
        if (!W) wrongType(way);
        copyFrom(way);
        return *this;
    }

    // Copy assignment operator for FeatureBase<false, false, true> (Relation)
    FeatureBase& operator=(const FeatureBase<RelationPtr,false, false, true>& relation)
    {
        if (!R) wrongType(relation);
        copyFrom(relation);
        return *this;
    }

    /// @name Type & Identity
    /// @{

    FeatureType type() const noexcept;

    /// @brief Returns the ID of this Feature
    ///
    /// @return the ID of the Feature
    int64_t id() const noexcept
    {
        return isAnonymousNode() ? anonymousNode_.id : feature_.ptr.id();
    }

    /// @brief Returns `true` if this Feature is a @ref Node.
    ///
    bool isNode() const noexcept
    {
        return (N && (extendedType() & 6) == 0) || (!W && !R);
    }

    bool isAnonymousNode() const noexcept
    {
        return N && (extendedType() & 1);
    }

    /*
    bool isFeatureNode() const noexcept
    {
        return N && (extendedType() & 1) == 0;
    }
    */

    /*
    bool isFeature() const noexcept
    {
        return !N || (extendedType() & 1) == 0;
    }
    */

    /// @brief Returns `true` if this Feature is a @ref Way.
    ///
    bool isWay() const noexcept
    {
        return (W && (extendedType() == 2)) || (!N && !R);
    }

    /// @brief Returns `true` if this Feature is a @ref Relation.
    ///
    bool isRelation() const noexcept
    {
        return (R && (extendedType() == 4)) || (!N && !W);
    }

    /// @brief Returns `true` if this Feature is an area (represented
    /// by either a @ref Way or @ref Relation)
    ///
    bool isArea() const noexcept
    {
        return (W || R) && !isNode() && feature_.ptr.isArea();
    }

    bool belongsToRelation() const noexcept
    {
        if(isAnonymousNode()) return false;
        return feature_.ptr.belongsToRelation();
    }

    /// @brief If this Feature was returned by a call to @ref Feature::members()
    /// of a Relation, returns this Feature's role in that Relation.
    ///
    /// @return the Feature's role (or an empty string)
    StringValue role() const noexcept
    {
        if(isAnonymousNode()) return {};
        return feature_.role;
    }

    /// @}
    /// @name Display
    /// @{

    /// @brief Depending on the type, returns `node`, `way`, or `relation`
    ///
    /// @return pointer null-terminated constant string
    ///
    const char* typeName() const noexcept
    {
        if (isNode()) return "node";
        if (isWay()) return "way";
        assert(isRelation());
        return "relation";
    }

    /// @brief Formats the string representation (e.g. `node/123456`)
    /// of this Feature into the provided buffer. Appends a
    /// null-terminator at the end of the output, and returns
    /// a pointer to the null-terminator, allowing easy
    /// concatenation.
    ///
    /// @param buf pointer to a `char` array of sufficient size
    /// @return a pointer to the null-terminator
    ///
    char* format(char* buf) const noexcept
    {
        const char* s = typeName();
        char* p = buf;
        while (*s) *p++ = *s++;
        *p = '/';
        return Format::integer(p + 1, id());
    }

    std::string toString() const
    {
        char buf[32];
        format(buf);
        return {buf};
    }

    /// @}
    /// @name Tags
    /// @{

    /// @brief Obtains the tag value for the given key.
    ///
    /// @return the tag's value (or an empty string
    ///         if the feature doesn't have a tag with this key)
    TagValue operator[](std::string_view key) const noexcept
    {
        if(isAnonymousNode()) return {};     // empty string
        const TagsRef tags = feature_.ptr.tagsOld();
        const TagBits val = tags.getKeyValue(key, store_.ptr()->strings());
        return tags.tagValue(val, store_.ptr()->strings());
    }

    Tags tags() const noexcept;

    /// @}
    /// @name Geometry
    /// @{

    /// @brief Returns the bounding box of this Feature.
    ///
    /// @box The Feature's bounding box (in Mercator projection)
    ///
    Box bounds() const noexcept
    {
        if (isNode())
        {
            return isAnonymousNode() ? Box(anonymousNode_.xy) : NodePtr(feature_.ptr).bounds();
        }
        return WayPtr(feature_.ptr).bounds();   // TODO: make Feature2DPtr
    }

    /// @brief Returns the Mercator-projected x/y coordinate of a Node,
    /// or the center point of the bounding box for a Way or Relation.
    ///
    Coordinate xy() const noexcept
    {
        if (isNode())
        {
            return isAnonymousNode() ? anonymousNode_.xy : NodePtr(feature_.ptr).xy();
        }
        return WayPtr(feature_.ptr).bounds().center();   // TODO: make Feature2DPtr
    }

    /// @brief Returns the Mercator-projected x-coordinate of a Node,
    /// or the horizontal midpoint of the bounding box for a Way or Relation.
    ///
    int32_t x() const noexcept { return xy().x; }

    /// @brief Returns the Mercator-projected y-coordinate of a Node,
    /// or the vertical midpoint of the bounding box for a Way or Relation.
    ///
    int32_t y() const noexcept { return xy().y; }
    
    double lon() const noexcept 
    { 
        return Mercator::lon100ndFromX(x()); 
    }

    double lat() const noexcept
    {
        return Mercator::lat100ndFromY(y());
    }

    /// Calculates the centroid of this Feature
    ///
    /// @return the Feature's centroid (in Mercator projection)
    /// @throws QueryException if one or more tiles that contain
    ///   the geometry of a Relation are missing
    Coordinate centroid() const
    {
        // TODO: Can this throw for a relation if tiles are missing?
        if (isNode())
        {
            return xy();
        }
        else if (isWay())
        {
            // TODO
            return Centroid::ofWay(WayRef(feature_.ptr));
        }
        else
        {
            assert(isRelation());
            return Centroid::ofRelation(store_.ptr(), RelationRef(feature_.ptr));
        }
    }

    /// @brief Measures the area of a feature
    ///
    /// @return area (in square meters), or `0` if the feature is not polygonal
    double area() const;

    /// @brief Measures the length of a feature.
    ///
    /// @return length (in meters), or `0` if the feature is not lineal
    double length() const;

    /// @}
    /// @name Related Features
    /// @{

    Nodes nodes() const;
    Nodes nodes(const char* query) const;
    Features members() const;
    Features members(const char* query) const;
    Features parents() const;
    Features parents(const char* query) const;

    /// @}
    /// @name Access to the Low-Level API
    /// @{

    Ptr ptr() const noexcept
    {
        return isAnonymousNode() ? Ptr(nullptr) : feature_.ptr;
    }

    FeatureStore* store() const noexcept
    {
        return store_.ptr();
    }

    /// @}

private:
    //! @cond
    enum class ExtendedFeatureType
    {
        NODE = 0,
        ANONYMOUS_NODE = 1,
        WAY = 2,
        RELATION = 4
    };

    int extendedType() const { return store_.flags(); }

    template <typename OtherPtr, bool OtherN, bool OtherW, bool OtherR>
    void wrongType(const FeatureBase<OtherPtr, OtherN, OtherW, OtherR>& other)
    {
        throw std::runtime_error(std::string("Attempt to assign ") + 
            other.typeName() + " to " + typeName());
    }

    template <typename OtherPtr, bool OtherN, bool OtherW, bool OtherR>
    void copyFrom(const FeatureBase<OtherPtr, OtherN, OtherW, OtherR>& other)
    {
        store_ = other.store_;
        if (N)
        {
            if (other.isAnonymousNode())
            {
                anonymousNode_.id = other.anonymousNode_.id;
                anonymousNode_.xy = other.anonymousNode_.xy;
            }
            else
            {
                feature_.ptr = other.feature_.ptr;
                feature_.role = other.feature_.role;
            }
        }
        else
        {
            assert(!other.isAnonymousNode());
            feature_.ptr = other.feature_.ptr;
            feature_.role = other.feature_.role;
        }
    }

    TaggedPtr<FeatureStore,3> store_;
    union
    {
        struct 
        {
            FeaturePtr ptr;
            StringValue role;
        }
        feature_;
        struct 
        {
            int64_t id;
            Coordinate xy;
        } 
        anonymousNode_;
    };
    //! @endcond
};

} // namespace geodesk::detail

/// \endinternal

