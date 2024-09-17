// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include "feature/FeaturePtr.h"
#include "feature/FeatureStore.h"
#include "geom/Box.h"

namespace geodesk
{
class Feature;
class Nodes;
class Ways;
class Relations;
}

namespace geodesk {

///
/// @brief A collection of geographic features.
///
/// A Features object isn't a classical container; it doesn't actually
/// hold any Feature objects, but merely describes which features should
/// be fetched from a Geographic Object Library. A query is only
/// executed whenever the Features object is iterated, assigned to a container,
/// or when one of its scalar functions (such as count() or length()) is called.
/// This also means that query results are not cached: If you call count()
/// prior to iterating over the Features, *two* queries will be executed.
///
/// Features objects are lightweight and suitable for passing by value.
/// They are threadsafe and can be freely shared among different threads.
/// Their various matchers and filters (and associated resources such as
/// temporary indexes) are shared via refcounting.
///
/// To start working with a GOL, simply create a `Features` object
/// with the path of its file (the `.gol` extension may be omitted):
///
/// ```
/// Features world("path/to/planet.gol");
/// ```
/// This creates a FeatureStore that manages access to `planet.gol`.
/// The `world` object now represents a collection of all features
/// stored in the GOL (It is legal to create multiple `Features`
/// objects that refer to the same GOL -- they will share the same
/// FeatureStore).
///
/// To obtain subsets, apply filters using `()` or by calling a
/// named method -- or intersect multiple `Features` object with `&`:
///
/// ```
/// Features hotels = world("na[tourism=hotel]");  // All hotels
/// Features inParis = world.within(paris);        // All features in Paris
///
/// // Only hotels in Paris:
/// Features hotelsInParis = hotels.within(paris);
/// // or...
/// Features hotelsInParis = hotels & inParis;
/// ```
///
/// Applying a filter creates a copy of the Features object, with
/// the additional constraint. The original Features object is unaffected.
///
/// To retrieve the actual features:
///
/// ```
/// // By iterating:
/// for(Feature hotel : hotelsInParis)
/// {
///     std::cout << hotel["name"] << std::endl;
/// }
///
/// // As a vector:
/// std::vector<Feature> hotelList = hotelsInParis;
///
/// // The one and only:
/// Feature paris = world("a[boundary=administrative]"
///      "[admin_level=8][name=Paris]").one();
///
/// // The first (if any):
/// std::optional<Feature> anyHotel = hotels.first();
/// ```
///
/// Features has three subclasses: Nodes, Ways and Relations, which contain
/// only Node, Way and Relation objects. Assigning a Features object to a
/// subclass object implicitly filters the collection by type (Assigning
/// a subtype to an incompatible type results in an empty collection):
///
/// ```
/// Features world("world");
///
/// Relations busRoutes =                  // Only relations that
///     world("[type=route][route=bus]")   // are tagged as bus routes
///
/// Nodes nodes = busRoutes;               // empty collection
/// ```
///
/// Upon destruction of last of the Features objects that refer to the
/// same FeatureStore, the Geographic Object Library is automatically
/// closed and all of the FeatureStore's resources are released (This
/// differs from [GeoDesk for Java](https://docs.geodesk.com/java/libraries#closing-a-library),
/// which requires GOLs to be closed explicitly).
///
/// **Important:** Once a GOL has been closed, all Feature objects retrieved
/// via queries to that GOL are no longer valid; calling any of their
/// methods will result in undefined behavior (The same applies to Tags,
/// Tag, TagValue and StringValue objects).
///
class Features
{
public:
    /// @brief Creates a collection that contains all features in the
    /// given Geographic Object Library
    ///
    /// @param golFile path of the GOL (`.gol` extension may be omitted)
    ///
    Features(const char* golFile);

    /// @brief Creates a collection with all the features in
    /// the other collection.
    ///
    Features(const Features& other);

    /// @{
    /// @name State and Membership

    /// @brief Returns `true` if this collection contains
    /// at least one Feature.
    ///
    operator bool() const;

    /// @brief Returns `true` if this collection is empty.
    ///
    bool operator!() const;

    /// @brief Returns `true` if this collection contains
    /// the given Feature.
    ///
    bool contains(const Feature& feature) const;

    /// @}
    /// @{
    /// @name Filtering by type and tags

    /// @brief Only features that match the given query.
    ///
    /// @param query a query in <a href="https://docs.geodesk.com/goql">GOQL</a> format
    ///
    /// @throws QueryException if the query is malformed.
    ///
    Features operator()(const char* query) const;

    ///
    /// @brief Only nodes.
    ///
    Nodes nodes() const;

    ///
    /// @brief Only nodes that match the given query.
    ///
    /// @param query a query in <a href="https://docs.geodesk.com/goql">GOQL</a> format
    ///
    /// @throws QueryException if the query is malformed.
    ///
    Nodes nodes(const char* query);

    ///
    /// @brief Only ways.
    ///
    Ways ways() const;

    ///
    /// @brief Only ways that match the given query.
    ///
    /// @param query a query in <a href="https://docs.geodesk.com/goql">GOQL</a> format
    ///
    /// @throws QueryException if the query is malformed.
    ///
    Ways ways(const char* query) const;

    ///
    /// @brief Only relations.
    ///
    Relations relations() const;

    ///
    /// @brief Only relations that match the given query.
    ///
    /// @param query a query in <a href="https://docs.geodesk.com/goql">GOQL</a> format
    ///
    /// @throws QueryException if the query is malformed.
    ///
    Relations relations(const char* query) const;

    /// @}
    /// @name Retrieving Features
    /// @{

    /// @brief Returns the first Feature in this collection, or `std::nullopt` if the
    /// collection is empty.
    ///
    /// Only Nodes of a Way and member Features of a Relation are ordered; for all
    /// other collections, first() returns an arbitrary Feature.
    ///
    /// @return the first `Feature`, or `std::nullopt`
    ///
    std::optional<Feature> first() const;

    /// @brief Returns the one and only Feature in this collection.
    ///
    /// @return the sole Feature
    ///
    /// @throws QueryException if this collection is empty or contains more
    /// than one Feature
    ///
    Feature one() const;

    /// @brief Returns a `std::vector` with the Feature objects in this collection.
    ///
    operator std::vector<Feature>() const;

    /// @brief Returns a `std::vector` with the FeaturePtr pointers that
    /// refer to the Feature objects in this collection.
    ///
    operator std::vector<FeaturePtr>() const;

    /// @}
    /// @name Scalar Queries
    /// @{

    /// @brief Returns the total number of features in this collection.
    ///
    uint64_t count() const;

    /// @brief Calculates the total length (in meters) of the features
    /// in this collection.
    ///
    double length() const;

    /// @brief Calculates the total area (in square meters) of the features
    /// in this collection.
    ///
    double area() const;

    /// @}
    /// @name Spatial Filters
    /// @{

    /// @brief Only features whose bounding box intersects
    /// the given bounding box.
    ///
    /// @param box
    Features operator()(const Box& box) const;

    /// @brief Only features whose bounding box contains
    /// the given Coordinate.
    ///
    /// @param xy
    Features operator()(Coordinate xy) const;

    /// @brief Only features whose geometry intersects with the
    /// given Feature (short form of intersecting())
    ///
    Features operator()(const Feature& feature) const;

    /// @brief Only features whose geometry intersects with the
    /// given Feature.
    ///
    /// @throws QueryException if one or more tiles that contain
    ///   the geometry of a Relation are missing
    ///
    Features intersecting(const Feature& feature) const;

    /// @brief Only features that lie entirely inside the geometry
    /// of the given Feature.
    ///
    /// @throws QueryException if one or more tiles that contain
    ///   the geometry of a Relation are missing
    ///
    Features within(const Feature& feature) const;

    /// @brief Only features whose geometry contains the
    /// given Feature.
    ///
    /// @throws QueryException if one or more tiles that contain
    ///   the geometry of a Relation are missing
    ///
    Features containing(const Feature& feature) const;

    /// @brief Only features whose geometry contains the
    /// given Coordinate.
    ///
    Features containing(Coordinate xy) const;

    /// @brief Only features whose closest point lies within
    /// `distance` meters of `xy`.
    ///
    /// @param distance the maximum distance (in meters)
    /// @param xy thew center of the search radius
    ///
    Features maxMetersFrom(double distance, Coordinate xy) const;

    /// @}
    /// @name Topological Filters
    /// @{

    /// @brief Only nodes that belong to the given Way.
    ///
    Features nodesOf(const Feature& feature) const;

    /// @brief Only features that belong to the given Relation.
    ///
    Features membersOf(const Feature& feature) const;

    /// @brief Only features that are parent relations of the
    /// given Feature (or parent ways of the given Node).
    ///
    Features parentsOf(const Feature& feature) const;

    /// @brief Only features that share a common node with
    /// the given Feature.
    ///
    Features connectedTo(const Feature& feature) const;

    /// @}
    /// @name Filtering with Predicate
    /// @{

    /// @brief Only features that match the given predicate.
    ///
    /// @param predicate A callable object (e.g., lambda,
    ///  function pointer, or functor) that defines the
    ///  filtering logic. The callable must accept a
    ///  Feature and return a `bool`.
    ///
    /// **Important:** The provided predicate must be
    /// thread-safe, as it may be invoked concurrently.
    ///
    /// ```
    /// // Find all parks whose area is at least 1 km²
    /// // (one million square meters)
    ///
    /// Features parks = world("a[leisure=park]");
    /// Features largeParks = parks.filter([](Feature park)
    ///     { return park.area() > 1'000'000; });
    /// ```
    template <typename Predicate>
    Features filter(Predicate predicate) const;

    /// @}
    /// @name Access to the Low-Level API
    /// @{

    /// @brief Returns a pointer to the FeatureStore
    /// which contains the features in this collection.
    ///
    FeatureStore* store() const noexcept;

    /// @}
};

} // namespace geodesk



