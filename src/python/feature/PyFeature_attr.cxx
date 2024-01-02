// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

/* C++ code produced by gperf version 3.1 */
/* Command-line: 'C:\\dev\\geodesk-py\\tools\\gperf' -L C++ -t --class-name=PyFeature_AttrHash --lookup-function-name=lookup PyFeature_attr.txt  */
/* Computed positions: -k'1,$' */

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

#line 1 "PyFeature_attr.txt"

// Keywords
#include <assert.h>

#line 6 "PyFeature_attr.txt"
struct Attribute { const char *name; int index; };

#define TOTAL_KEYWORDS 26
#define MIN_WORD_LENGTH 1
#define MAX_WORD_LENGTH 14
#define MIN_HASH_VALUE 1
#define MAX_HASH_VALUE 38
/* maximum key range = 38, duplicates = 0 */

class PyFeature_AttrHash
{
private:
  static inline unsigned int hash (const char *str, size_t len);
public:
  static struct Attribute *lookup (const char *str, size_t len);
};

inline unsigned int
PyFeature_AttrHash::hash (const char *str, size_t len)
{
  static unsigned char asso_values[] =
    {
      39, 39, 39, 39, 39, 39, 39, 39, 39, 39,
      39, 39, 39, 39, 39, 39, 39, 39, 39, 39,
      39, 39, 39, 39, 39, 39, 39, 39, 39, 39,
      39, 39, 39, 39, 39, 39, 39, 39, 39, 39,
      39, 39, 39, 39, 39, 39, 39, 39, 39, 39,
      39, 39, 39, 39, 39, 39, 39, 39, 39, 39,
      39, 39, 39, 39, 39, 39, 39, 39, 39, 39,
      39, 39, 39, 39, 39, 39, 39, 39, 39, 39,
      39, 39, 39, 39, 39, 39, 39, 39, 39, 39,
      39, 39, 39, 39, 39, 39, 39, 10, 20, 30,
       0,  0, 39, 15, 15,  0, 39, 39,  0,  5,
       5, 25, 15, 39,  0,  0, 15, 39, 39, 10,
       5,  0, 39, 39, 39, 39, 39, 39, 39, 39,
      39, 39, 39, 39, 39, 39, 39, 39, 39, 39,
      39, 39, 39, 39, 39, 39, 39, 39, 39, 39,
      39, 39, 39, 39, 39, 39, 39, 39, 39, 39,
      39, 39, 39, 39, 39, 39, 39, 39, 39, 39,
      39, 39, 39, 39, 39, 39, 39, 39, 39, 39,
      39, 39, 39, 39, 39, 39, 39, 39, 39, 39,
      39, 39, 39, 39, 39, 39, 39, 39, 39, 39,
      39, 39, 39, 39, 39, 39, 39, 39, 39, 39,
      39, 39, 39, 39, 39, 39, 39, 39, 39, 39,
      39, 39, 39, 39, 39, 39, 39, 39, 39, 39,
      39, 39, 39, 39, 39, 39, 39, 39, 39, 39,
      39, 39, 39, 39, 39, 39, 39, 39, 39, 39,
      39, 39, 39, 39, 39, 39
    };
  return len + asso_values[static_cast<unsigned char>(str[len - 1])] + asso_values[static_cast<unsigned char>(str[0])];
}

struct Attribute *
PyFeature_AttrHash::lookup (const char *str, size_t len)
{
  static struct Attribute wordlist[] =
    {
      {""},
#line 33 "PyFeature_attr.txt"
      {"y", 25},
#line 12 "PyFeature_attr.txt"
      {"id", 4},
#line 29 "PyFeature_attr.txt"
      {"str", 21},
#line 27 "PyFeature_attr.txt"
      {"role", 19},
#line 28 "PyFeature_attr.txt"
      {"shape", 20},
#line 17 "PyFeature_attr.txt"
      {"is_way", 9},
#line 14 "PyFeature_attr.txt"
      {"is_node", 6},
#line 20 "PyFeature_attr.txt"
      {"lon", 12},
      {""},
#line 23 "PyFeature_attr.txt"
      {"nodes", 15},
#line 32 "PyFeature_attr.txt"
      {"x", 24},
#line 22 "PyFeature_attr.txt"
      {"members", 14},
#line 24 "PyFeature_attr.txt"
      {"num", 16},
#line 15 "PyFeature_attr.txt"
      {"is_placeholder", 7},
      {""},
#line 16 "PyFeature_attr.txt"
      {"is_relation", 8},
#line 13 "PyFeature_attr.txt"
      {"is_area", 5},
#line 18 "PyFeature_attr.txt"
      {"lat", 10},
#line 30 "PyFeature_attr.txt"
      {"tags", 22},
      {""},
#line 19 "PyFeature_attr.txt"
      {"length", 11},
#line 26 "PyFeature_attr.txt"
      {"parents", 18},
#line 21 "PyFeature_attr.txt"
      {"map", 13},
#line 8 "PyFeature_attr.txt"
      {"area", 0},
      {""},
#line 9 "PyFeature_attr.txt"
      {"bounds", 1},
#line 11 "PyFeature_attr.txt"
      {"geojson", 3},
#line 31 "PyFeature_attr.txt"
      {"wkt", 23},
      {""}, {""}, {""}, {""},
#line 25 "PyFeature_attr.txt"
      {"osm_type", 17},
      {""}, {""}, {""}, {""},
#line 10 "PyFeature_attr.txt"
      {"centroid", 2}
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
