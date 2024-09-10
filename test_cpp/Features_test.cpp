// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#include <iostream>
#include <memory>
#include <string_view>
#include <catch2/catch_test_macros.hpp>
#include "api/Features.h"
#include "api/Ways.h"

using namespace geodesk;


TEST_CASE("Features")
{
	Features world(R"(c:\geodesk\tests\w3.gol)");
	Feature france = world("a[boundary=administrative][admin_level=2][name=France]").one();
	Feature paris = world("a[boundary=administrative][admin_level=8][name=Paris]")(france).one();
	std::cout << "Population of Paris: " << paris["population"] << std::endl;
	REQUIRE(paris["name"] == "Paris");
	REQUIRE(paris["population"] > 2'000'000);
	/*
	Feature usa = world("a[boundary=administrative][admin_level=2][name='United States']").one();
	Features buildings = world("a[building]");
	printf("%lld buildings in the US\n", buildings.within(usa).count());
	*/
	Ways streets = world("[highway=primary]");
	std::cout << "There are " << streets.within(paris).count() << " streets" << std::endl;
	for (Way street : streets.within(paris))
	{
		std::cout << street["name"] << std::endl;
	}
}

TEST_CASE("Features2")
{
	Features world(R"(c:\geodesk\tests\w3.gol)");
	Feature usa = world("a[boundary=administrative][admin_level=2][name='United States']").one();
	Features buildings = world("a[building]");
	Features usaBuildings = buildings(usa);
	for(int i=0; i<10; i++)
	{
		std::cout << usaBuildings.count() << " buildings" << std::endl;
	}
}