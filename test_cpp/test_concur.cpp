#include <iostream>
#include <vector>
#include <string>
#include <functional>
#include <cstdint>
#include <catch2/catch_test_macros.hpp>
#include <geodesk/feature/Features.h>
#include <geodesk/feature/Ways.h>

using namespace geodesk;

// Structure to hold test information
struct GeodeskConcurTest
{
    std::string name;
    std::function<int64_t()> function;
};

// Vector to hold all the tests
std::vector<GeodeskConcurTest> geodesk_concur_tests;

// Function to register a test
void registerConcurTest(const std::string& name, std::function<int64_t()> func)
{
    geodesk_concur_tests.emplace_back(name, func);
}

// Macro to define a test
#define GEODESK_TEST(test_name)                                      \
int64_t test_name##_impl();                                      \
struct test_name##_registrar                                     \
{                                                                \
test_name##_registrar()                                       \
{                                                            \
registerConcurTest(#test_name, test_name##_impl);             \
}                                                            \
} test_name##_registrar_instance;                                \
int64_t test_name##_impl()

static Features world("c:\\geodesk\\tests\\monaco.gol");


GEODESK_TEST(italian_restaurant_count)
{
    return world("na[amenity=restaurant][cuisine=italian]").count();
}

/*
GEODESK_TEST(centroid_hash)
{
    int64_t hash = 0;
    for (auto f: world)
    {
        Coordinate c = f.centroid();
        hash ^= c.x;
        hash ^= c.y;
    }
    return hash;
}
*/

GEODESK_TEST(xy_hash)
{
    int64_t hash = 0;
    for (auto f: world)
    {
        hash ^= f.x();
        hash ^= f.y();
    }
    return hash;
}

/*
GEODESK_TEST(lonlat_100nd_hash)
{
    int64_t hash = 0;
    for (auto f: world)
    {
        // hash ^= (long)(f.lon() * 10000000);
        // hash ^= (long)(f.lat() * 10000000);
        hash ^= (long)(Mercator.lonPrecision7fromX(f.x()) * 10000000);
        hash ^= (long)(Mercator.latPrecision7fromY(f.y()) * 10000000);
    }
    return hash;
}
 */

GEODESK_TEST(id_hash)
{
    int64_t hash = 0;
    for (auto f: world)
    {
        hash ^= f.id();
    }
    return hash;
}


TEST_CASE("concur")
{
    for(auto test: geodesk_concur_tests)
    {
        std::cout << test.name << ": " << test.function() << std::endl;
    }
}