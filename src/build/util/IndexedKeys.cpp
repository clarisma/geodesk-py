#include "IndexedKeys.h"

IndexedKeys::IndexedKeys()
{

}

const char* const IndexedKeys::DEFAULT_INDEXED_KEY_STRINGS[] =
{
	"place",
	"highway",
	"railway",
	"aeroway",
	"aerialway",
	"tourism",
	"amenity",
	"shop",
	"craft",
	"power",
	"industrial",
	"man_made",
	"leisure",
	"landuse",
	"waterway",
	"natural",
	"geological",
	"military",
	"historic",
	"healthcare",
	"office",
	"emergency",
	"building",
	"boundary",
	"building:part",
	"telecom",
	"communication",
	"route",
};

const uint8_t DEFAULT_INDEXED_KEY_CATEGORIES[] =
{
	1, // place
	2, // highway
	3, // railway
	4, // aeroway
	5, // aerialway
	6, // tourism
	7, // amenity
	8, // shop
	9, // craft
	10, // power
	11, // industrial
	12, // man_made
	13, // leisure
	14, // landuse
	15, // waterway
	16, // natural &
	16, // geological
	17, // military
	18, // historic
	19, // healthcare
	20, // office
	21, // emergency
	22, // building
	23, // boundary
	24, // building:part
	25, // telecom &
	25, // communication
	26, // route
};
