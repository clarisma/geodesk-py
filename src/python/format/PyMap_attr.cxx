// Copyright (c) 2023 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

/* C++ code produced by gperf version 3.1 */
/* Command-line: 'C:\\dev\\geodesk-py\\tools\\gperf' -L C++ -t --class-name=PyMap_AttrHash --lookup-function-name=lookup PyMap_attr.txt  */
/* Computed positions: -k'3,5' */

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

#line 1 "PyMap_attr.txt"

// Keywords
#include <assert.h>

#line 6 "PyMap_attr.txt"
struct Attr { const char *name; int index; };

#define TOTAL_KEYWORDS 29
#define MIN_WORD_LENGTH 4
#define MAX_WORD_LENGTH 22
#define MIN_HASH_VALUE 6
#define MAX_HASH_VALUE 42
/* maximum key range = 37, duplicates = 0 */

class PyMap_AttrHash
{
private:
  static inline unsigned int hash (const char *str, size_t len);
public:
  static struct Attr *lookup (const char *str, size_t len);
};

inline unsigned int
PyMap_AttrHash::hash (const char *str, size_t len)
{
  static unsigned char asso_values[] =
    {
      43, 43, 43, 43, 43, 43, 43, 43, 43, 43,
      43, 43, 43, 43, 43, 43, 43, 43, 43, 43,
      43, 43, 43, 43, 43, 43, 43, 43, 43, 43,
      43, 43, 43, 43, 43, 43, 43, 43, 43, 43,
      43, 43, 43, 43, 43, 43, 43, 43, 43, 43,
      43, 43, 43, 43, 43, 43, 43, 43, 43, 43,
      43, 43, 43, 43, 43, 30, 43, 15, 43, 43,
      43, 43, 43, 43, 10, 43, 43, 43, 43,  5,
      43, 43,  0, 43, 43, 43, 43, 43, 43, 43,
      43, 43, 43, 43, 43,  0, 43,  0, 43, 43,
      43, 43, 43, 43,  0,  0, 43, 10, 10, 10,
      20,  0, 43, 43, 15,  0,  5, 43, 43, 43,
       0, 43,  5, 43, 43, 43, 43, 43, 43, 43,
      43, 43, 43, 43, 43, 43, 43, 43, 43, 43,
      43, 43, 43, 43, 43, 43, 43, 43, 43, 43,
      43, 43, 43, 43, 43, 43, 43, 43, 43, 43,
      43, 43, 43, 43, 43, 43, 43, 43, 43, 43,
      43, 43, 43, 43, 43, 43, 43, 43, 43, 43,
      43, 43, 43, 43, 43, 43, 43, 43, 43, 43,
      43, 43, 43, 43, 43, 43, 43, 43, 43, 43,
      43, 43, 43, 43, 43, 43, 43, 43, 43, 43,
      43, 43, 43, 43, 43, 43, 43, 43, 43, 43,
      43, 43, 43, 43, 43, 43, 43, 43, 43, 43,
      43, 43, 43, 43, 43, 43, 43, 43, 43, 43,
      43, 43, 43, 43, 43, 43, 43, 43, 43, 43,
      43, 43, 43, 43, 43, 43
    };
  unsigned int hval = len;

  switch (hval)
    {
      default:
        hval += asso_values[static_cast<unsigned char>(str[4])];
      /*FALLTHROUGH*/
      case 4:
      case 3:
        hval += asso_values[static_cast<unsigned char>(str[2])];
        break;
    }
  return hval;
}

struct Attr *
PyMap_AttrHash::lookup (const char *str, size_t len)
{
  static struct Attr wordlist[] =
    {
      {""}, {""}, {""}, {""}, {""}, {""},
#line 36 "PyMap_attr.txt"
      {"weight", PyMap::WEIGHT},
#line 33 "PyMap_attr.txt"
      {"opacity", PyMap::OPACITY},
      {""},
#line 10 "PyMap_attr.txt"
      {"classname", PyMap::CLASSNAME},
#line 13 "PyMap_attr.txt"
      {"dash_array", PyMap::DASHARRAY},
#line 15 "PyMap_attr.txt"
      {"dash_offset", PyMap::DASHOFFSET},
#line 35 "PyMap_attr.txt"
      {"tooltip", PyMap::TOOLTIP},
#line 31 "PyMap_attr.txt"
      {"max_zoom", PyMap::MAX_ZOOM},
#line 16 "PyMap_attr.txt"
      {"fill", PyMap::FILL},
#line 14 "PyMap_attr.txt"
      {"dashOffset", PyMap::DASHOFFSET},
#line 8 "PyMap_attr.txt"
      {"attribution", PyMap::ATTRIBUTION},
#line 9 "PyMap_attr.txt"
      {"basemap", PyMap::BASEMAP},
#line 21 "PyMap_attr.txt"
      {"fillRule", PyMap::FILLRULE},
#line 22 "PyMap_attr.txt"
      {"fill_rule", PyMap::FILLRULE},
#line 18 "PyMap_attr.txt"
      {"fill_color", PyMap::FILLCOLOR},
#line 24 "PyMap_attr.txt"
      {"leaflet_url", PyMap::LEAFLET_URL},
#line 20 "PyMap_attr.txt"
      {"fill_opacity", PyMap::FILLOPACITY},
      {""},
#line 30 "PyMap_attr.txt"
      {"link", PyMap::LINK},
#line 25 "PyMap_attr.txt"
      {"leaflet_version", PyMap::LEAFLET_VERSION},
#line 19 "PyMap_attr.txt"
      {"fillOpacity", PyMap::FILLOPACITY},
      {""},
#line 27 "PyMap_attr.txt"
      {"line_cap", PyMap::LINECAP},
#line 29 "PyMap_attr.txt"
      {"line_join", PyMap::LINEJOIN},
#line 11 "PyMap_attr.txt"
      {"color", PyMap::COLOR},
#line 34 "PyMap_attr.txt"
      {"stroke", PyMap::STROKE},
#line 23 "PyMap_attr.txt"
      {"leaflet_stylesheet_url", PyMap::LEAFLET_STYLESHEET_URL},
#line 32 "PyMap_attr.txt"
      {"min_zoom", PyMap::MIN_ZOOM},
#line 17 "PyMap_attr.txt"
      {"fillColor", PyMap::FILLCOLOR},
      {""}, {""}, {""},
#line 28 "PyMap_attr.txt"
      {"lineJoin", PyMap::LINEJOIN},
#line 12 "PyMap_attr.txt"
      {"dashArray", PyMap::DASHARRAY},
      {""}, {""},
#line 26 "PyMap_attr.txt"
      {"lineCap", PyMap::LINECAP}
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
