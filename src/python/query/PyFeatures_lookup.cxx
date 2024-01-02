// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

/* C++ code produced by gperf version 3.1 */
/* Command-line: 'C:\\dev\\geodesk-py\\tools\\gperf' -L C++ -t --class-name=PyFeatures_AttrHash --lookup-function-name=lookup PyFeatures_attr.txt  */
/* Computed positions: -k'3-4' */

#if !((' ' == 32) && ('!' == 33) && ('"' == 34) && ('#' == 35) \
      && ('%' == 37) && ('&' == 38) && ('\'' == 39) && ('(' == 40) \
      && (')' == 41) && ('*' == 42) && ('+' == 43) && (',' == 44) \
      && ('-' == 45) && ('.' == 46) && ('/' == 47) && ('0' == 48) \
      && ('1' == 49) && ('2' == 50) && ('3' == 51) && ('4' == 52) \
      && ('5' == 53) && ('6' == 54) && ('7' == 55) && ('8' == 56) \
      && ('9' == 57) && (':' == 58) && (';' == 59) && ('<' == 60) \
      && ('=' == 61) && ('>' == 62) && ('?' == 63) && ('A' == 65) \
      && ('B' == 66) && ('C' == 67) && ('D' == 68) && ('E' == 69) \
      && ('F' == 70) && ('G' == 71) && ('H' == 72) && ('I' == 73) \
      && ('J' == 74) && ('K' == 75) && ('L' == 76) && ('M' == 77) \
      && ('N' == 78) && ('O' == 79) && ('P' == 80) && ('Q' == 81) \
      && ('R' == 82) && ('S' == 83) && ('T' == 84) && ('U' == 85) \
      && ('V' == 86) && ('W' == 87) && ('X' == 88) && ('Y' == 89) \
      && ('Z' == 90) && ('[' == 91) && ('\\' == 92) && (']' == 93) \
      && ('^' == 94) && ('_' == 95) && ('a' == 97) && ('b' == 98) \
      && ('c' == 99) && ('d' == 100) && ('e' == 101) && ('f' == 102) \
      && ('g' == 103) && ('h' == 104) && ('i' == 105) && ('j' == 106) \
      && ('k' == 107) && ('l' == 108) && ('m' == 109) && ('n' == 110) \
      && ('o' == 111) && ('p' == 112) && ('q' == 113) && ('r' == 114) \
      && ('s' == 115) && ('t' == 116) && ('u' == 117) && ('v' == 118) \
      && ('w' == 119) && ('x' == 120) && ('y' == 121) && ('z' == 122) \
      && ('{' == 123) && ('|' == 124) && ('}' == 125) && ('~' == 126))
/* The character set is not based on ISO-646.  */
#error "gperf generated tables don't work with this execution character set. Please report a bug to <bug-gperf@gnu.org>."
#endif

#line 1 "PyFeatures_attr.txt"

// Keywords
#include <assert.h>
#include <cstdint>
#include "python/filter/filters.h"
#include "PyFeatures.h"
#include "python/util/util.h"

#line 10 "PyFeatures_attr.txt"
struct PyFeaturesAttribute { const char *name; Python::AttrRef attr; };

#define TOTAL_KEYWORDS 48
#define MIN_WORD_LENGTH 3
#define MAX_WORD_LENGTH 15
#define MIN_HASH_VALUE 4
#define MAX_HASH_VALUE 81
/* maximum key range = 78, duplicates = 0 */

class PyFeatures_AttrHash
{
private:
  static inline unsigned int hash (const char *str, size_t len);
public:
  static struct PyFeaturesAttribute *lookup (const char *str, size_t len);
};

inline unsigned int
PyFeatures_AttrHash::hash (const char *str, size_t len)
{
  static unsigned char asso_values[] =
    {
      82, 82, 82, 82, 82, 82, 82, 82, 82, 82,
      82, 82, 82, 82, 82, 82, 82, 82, 82, 82,
      82, 82, 82, 82, 82, 82, 82, 82, 82, 82,
      82, 82, 82, 82, 82, 82, 82, 82, 82, 82,
      82, 82, 82, 82, 82, 82, 82, 82, 82, 82,
      82, 82, 82, 82, 82, 82, 82, 82, 82, 82,
      82, 82, 82, 82, 82, 82, 82, 82, 82, 82,
      82, 82, 82, 82, 82, 82, 82, 82, 82, 82,
      82, 82, 82, 82, 82, 82, 82, 82, 82, 82,
      82, 82, 82, 82, 82, 30, 82,  0, 60, 20,
       0,  5, 30,  5, 15, 15, 40, 82, 55,  0,
       0,  0, 50, 82,  5, 15, 20, 25, 10, 82,
       5,  5, 82, 82, 82, 82, 82, 82, 82, 82,
      82, 82, 82, 82, 82, 82, 82, 82, 82, 82,
      82, 82, 82, 82, 82, 82, 82, 82, 82, 82,
      82, 82, 82, 82, 82, 82, 82, 82, 82, 82,
      82, 82, 82, 82, 82, 82, 82, 82, 82, 82,
      82, 82, 82, 82, 82, 82, 82, 82, 82, 82,
      82, 82, 82, 82, 82, 82, 82, 82, 82, 82,
      82, 82, 82, 82, 82, 82, 82, 82, 82, 82,
      82, 82, 82, 82, 82, 82, 82, 82, 82, 82,
      82, 82, 82, 82, 82, 82, 82, 82, 82, 82,
      82, 82, 82, 82, 82, 82, 82, 82, 82, 82,
      82, 82, 82, 82, 82, 82, 82, 82, 82, 82,
      82, 82, 82, 82, 82, 82, 82, 82, 82, 82,
      82, 82, 82, 82, 82, 82
    };
  unsigned int hval = len;

  switch (hval)
    {
      default:
        hval += asso_values[static_cast<unsigned char>(str[3])];
      /*FALLTHROUGH*/
      case 3:
        hval += asso_values[static_cast<unsigned char>(str[2])];
        break;
    }
  return hval;
}

struct PyFeaturesAttribute *
PyFeatures_AttrHash::lookup (const char *str, size_t len)
{
  static struct PyFeaturesAttribute wordlist[] =
    {
      {""}, {""}, {""}, {""},
#line 35 "PyFeatures_attr.txt"
      {"load",              ATTR_METHOD(PyFeatures::load)},
      {""},
#line 36 "PyFeatures_attr.txt"
      {"update",            ATTR_METHOD(PyFeatures::update)},
      {""},
#line 23 "PyFeatures_attr.txt"
      {"one", ATTR_PROPERTY(PyFeatures::one)},
#line 12 "PyFeatures_attr.txt"
      {"area", ATTR_PROPERTY(PyFeatures::area)},
#line 22 "PyFeatures_attr.txt"
      {"nodes", ATTR_PROPERTY(PyFeatures::nodes)},
#line 19 "PyFeatures_attr.txt"
      {"length", ATTR_PROPERTY(PyFeatures::length)},
#line 39 "PyFeatures_attr.txt"
      {"connected_to",      ATTR_METHOD(filters::connected_to)},
#line 54 "PyFeatures_attr.txt"
      {"nodes_of",          ATTR_METHOD(filters::nodes_of)},
#line 31 "PyFeatures_attr.txt"
      {"timestamp", ATTR_PROPERTY(PyFeatures::timestamp)},
#line 53 "PyFeatures_attr.txt"
      {"nearest_to",        ATTR_METHOD(filters::nearest_to)},
      {""},
#line 18 "PyFeatures_attr.txt"
      {"indexed_keys", ATTR_PROPERTY(PyFeatures::indexed_keys)},
#line 55 "PyFeatures_attr.txt"
      {"overlaps",          ATTR_METHOD(filters::overlaps)},
#line 17 "PyFeatures_attr.txt"
      {"guid", ATTR_PROPERTY(PyFeatures::guid)},
#line 56 "PyFeatures_attr.txt"
      {"parents_of",        ATTR_METHOD(filters::parents_of)},
      {""},
#line 42 "PyFeatures_attr.txt"
      {"crosses",           ATTR_METHOD(filters::crosses)},
#line 33 "PyFeatures_attr.txt"
      {"wkt", ATTR_PROPERTY(PyFormatter::wkt)},
#line 32 "PyFeatures_attr.txt"
      {"ways", ATTR_PROPERTY(PyFeatures::ways)},
#line 14 "PyFeatures_attr.txt"
      {"first", ATTR_PROPERTY(PyFeatures::first)},
      {""},
#line 29 "PyFeatures_attr.txt"
      {"strings", ATTR_PROPERTY(PyFeatures::strings)},
#line 40 "PyFeatures_attr.txt"
      {"contains",          ATTR_METHOD(filters::contains)},
#line 34 "PyFeatures_attr.txt"
      {"auto_load",         ATTR_METHOD(PyFeatures::auto_load)},
#line 13 "PyFeatures_attr.txt"
      {"count", ATTR_PROPERTY(PyFeatures::count)},
#line 38 "PyFeatures_attr.txt"
      {"around",            ATTR_METHOD(filters::around)},
#line 41 "PyFeatures_attr.txt"
      {"contained_by",      ATTR_METHOD(filters::contained_by)},
#line 27 "PyFeatures_attr.txt"
      {"revision", ATTR_PROPERTY(PyFeatures::revision)},
      {""},
#line 46 "PyFeatures_attr.txt"
      {"intersects",        ATTR_METHOD(filters::intersects)},
      {""},
#line 37 "PyFeatures_attr.txt"
      {"ancestors_of",      ATTR_METHOD(filters::ancestors_of)},
#line 50 "PyFeatures_attr.txt"
      {"min_area",          ATTR_METHOD(filters::min_area)},
#line 20 "PyFeatures_attr.txt"
      {"list", ATTR_PROPERTY(PyFeatures::list)},
#line 52 "PyFeatures_attr.txt"
      {"min_length",        ATTR_METHOD(filters::min_length)},
#line 59 "PyFeatures_attr.txt"
      {"within",            ATTR_METHOD(filters::within)},
      {""},
#line 47 "PyFeatures_attr.txt"
      {"max_area",          ATTR_METHOD(filters::max_area)},
#line 58 "PyFeatures_attr.txt"
      {"with_role",         ATTR_METHOD(filters::with_role)},
#line 48 "PyFeatures_attr.txt"
      {"max_length",        ATTR_METHOD(filters::max_length)},
      {""},
#line 15 "PyFeatures_attr.txt"
      {"geojson", ATTR_PROPERTY(PyFormatter::geojson)},
#line 16 "PyFeatures_attr.txt"
      {"geojsonl", ATTR_PROPERTY(PyFormatter::geojsonl)},
#line 43 "PyFeatures_attr.txt"
      {"descendants_of",    ATTR_METHOD(filters::descendants_of)},
#line 49 "PyFeatures_attr.txt"
      {"max_meters_from",   ATTR_METHOD(filters::max_meters_from)},
      {""},
#line 57 "PyFeatures_attr.txt"
      {"touches",           ATTR_METHOD(filters::touches)},
#line 21 "PyFeatures_attr.txt"
      {"map", ATTR_PROPERTY(PyFeatures::map)},
      {""},
#line 28 "PyFeatures_attr.txt"
      {"shape", ATTR_PROPERTY(PyFeatures::shape)},
      {""}, {""},
#line 25 "PyFeatures_attr.txt"
      {"refcount", ATTR_PROPERTY(PyFeatures::refcount)},
      {""},
#line 24 "PyFeatures_attr.txt"
      {"properties", ATTR_PROPERTY(PyFeatures::properties)},
      {""}, {""},
#line 44 "PyFeatures_attr.txt"
      {"disjoint",          ATTR_METHOD(filters::disjoint)},
#line 26 "PyFeatures_attr.txt"
      {"relations", ATTR_PROPERTY(PyFeatures::relations)},
#line 30 "PyFeatures_attr.txt"
      {"tiles", ATTR_PROPERTY(PyFeatures::tiles)},
      {""}, {""}, {""}, {""},
#line 51 "PyFeatures_attr.txt"
      {"members_of",        ATTR_METHOD(filters::members_of)},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""},
#line 45 "PyFeatures_attr.txt"
      {"filter",            ATTR_METHOD(filters::pythonFilter)}
    };

  if (len <= MAX_WORD_LENGTH && len >= MIN_WORD_LENGTH)
    {
      unsigned int key = hash (str, len);

      if (key <= MAX_HASH_VALUE)
        {
          const char *s = wordlist[key].name;

          if (*str == *s && !strcmp (str + 1, s + 1))
            return &wordlist[key];
        }
    }
  return 0;
}
