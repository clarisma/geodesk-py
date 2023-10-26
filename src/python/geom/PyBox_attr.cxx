// Copyright (c) 2023 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

/* C++ code produced by gperf version 3.1 */
/* Command-line: 'C:\\dev\\geodesk-py\\tools\\gperf' -L C++ -t --class-name=PyBox_AttrHash --lookup-function-name=lookup PyBox_attr.txt  */
/* Computed positions: -k'1-2,$' */

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

#line 1 "PyBox_attr.txt"

// Keywords
#include <assert.h>

// Bit 0:	0 = GeoDesk Mercator, 1 = WGS-84 
// Bit 1:	0 = single value,     1 = copy min value to max
// Bits 8/9	0 = minX, 1= minY, 2 = maxX, 3 = maxY

#line 10 "PyBox_attr.txt"
struct Attribute { const char *name; int index; };

#define TOTAL_KEYWORDS 24
#define MIN_WORD_LENGTH 1
#define MAX_WORD_LENGTH 6
#define MIN_HASH_VALUE 1
#define MAX_HASH_VALUE 84
/* maximum key range = 84, duplicates = 0 */

class PyBox_AttrHash
{
private:
  static inline unsigned int hash (const char *str, size_t len);
public:
  static struct Attribute *lookup (const char *str, size_t len);
};

inline unsigned int
PyBox_AttrHash::hash (const char *str, size_t len)
{
  static unsigned char asso_values[] =
    {
      85, 85, 85, 85, 85, 85, 85, 85, 85, 85,
      85, 85, 85, 85, 85, 85, 85, 85, 85, 85,
      85, 85, 85, 85, 85, 85, 85, 85, 85, 85,
      85, 85, 85, 85, 85, 85, 85, 85, 85, 85,
      85, 85, 85, 85, 85, 85, 85, 85, 85, 85,
      85, 85, 85, 85, 85, 85, 85, 85, 85, 85,
      85, 85, 85, 85, 85, 85, 85, 85, 85, 85,
      85, 85, 85, 85, 85, 85, 85, 85, 85, 85,
      85, 85, 85, 85, 85, 85, 85, 85, 85, 85,
      85, 85, 85, 85, 85, 85, 85, 10,  0, 85,
      85, 25, 85, 85,  0,  0, 85, 85,  0,  0,
      10,  0,  0, 85,  0,  3, 25, 85, 85, 30,
       5,  0, 85, 85, 85, 85, 85, 85, 85, 85,
      85, 85, 85, 85, 85, 85, 85, 85, 85, 85,
      85, 85, 85, 85, 85, 85, 85, 85, 85, 85,
      85, 85, 85, 85, 85, 85, 85, 85, 85, 85,
      85, 85, 85, 85, 85, 85, 85, 85, 85, 85,
      85, 85, 85, 85, 85, 85, 85, 85, 85, 85,
      85, 85, 85, 85, 85, 85, 85, 85, 85, 85,
      85, 85, 85, 85, 85, 85, 85, 85, 85, 85,
      85, 85, 85, 85, 85, 85, 85, 85, 85, 85,
      85, 85, 85, 85, 85, 85, 85, 85, 85, 85,
      85, 85, 85, 85, 85, 85, 85, 85, 85, 85,
      85, 85, 85, 85, 85, 85, 85, 85, 85, 85,
      85, 85, 85, 85, 85, 85, 85, 85, 85, 85,
      85, 85, 85, 85, 85, 85
    };
  unsigned int hval = len;

  switch (hval)
    {
      default:
        hval += asso_values[static_cast<unsigned char>(str[1])];
      /*FALLTHROUGH*/
      case 1:
        hval += asso_values[static_cast<unsigned char>(str[0])];
        break;
    }
  return hval + asso_values[static_cast<unsigned char>(str[len - 1])];
}

struct Attribute *
PyBox_AttrHash::lookup (const char *str, size_t len)
{
  static struct Attribute wordlist[] =
    {
      {""},
#line 35 "PyBox_attr.txt"
      {"y", (1 << 8) | 2},
      {""}, {""},
#line 25 "PyBox_attr.txt"
      {"miny", (1 << 8) | 0},
      {""},
#line 12 "PyBox_attr.txt"
      {"bottom", (1 << 8) | 0},
#line 29 "PyBox_attr.txt"
      {"s", (1 << 8) | 1},
#line 30 "PyBox_attr.txt"
      {"south", (1 << 8) | 1},
#line 24 "PyBox_attr.txt"
      {"minx", (0 << 8) | 0},
      {""},
#line 34 "PyBox_attr.txt"
      {"x", (0 << 8) | 2},
      {""},
#line 17 "PyBox_attr.txt"
      {"lon", (0 << 8) | 3},
#line 21 "PyBox_attr.txt"
      {"maxy", (3 << 8) | 0},
#line 27 "PyBox_attr.txt"
      {"north", (3 << 8) | 1},
#line 23 "PyBox_attr.txt"
      {"minlon", (0 << 8) | 1},
      {""}, {""},
#line 20 "PyBox_attr.txt"
      {"maxx", (2 << 8) | 0},
      {""},
#line 26 "PyBox_attr.txt"
      {"n", (3 << 8) | 1},
      {""}, {""}, {""}, {""},
#line 19 "PyBox_attr.txt"
      {"maxlon", (2 << 8) | 1},
      {""},
#line 31 "PyBox_attr.txt"
      {"top", (3 << 8) | 0},
      {""},
#line 28 "PyBox_attr.txt"
      {"right", (2 << 8) | 0},
#line 22 "PyBox_attr.txt"
      {"minlat", (1 << 8) | 1},
      {""}, {""}, {""}, {""}, {""}, {""},
#line 15 "PyBox_attr.txt"
      {"lat", (1 << 8) | 3},
      {""}, {""},
#line 18 "PyBox_attr.txt"
      {"maxlat", (3 << 8) | 1},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
#line 13 "PyBox_attr.txt"
      {"e", (2 << 8) | 1},
      {""}, {""},
#line 16 "PyBox_attr.txt"
      {"left", (0 << 8) | 0},
      {""}, {""}, {""}, {""}, {""}, {""},
#line 32 "PyBox_attr.txt"
      {"w", (0 << 8) | 1},
      {""}, {""},
#line 14 "PyBox_attr.txt"
      {"east", (2 << 8) | 1},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""},
#line 33 "PyBox_attr.txt"
      {"west", (0 << 8) | 1}
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
