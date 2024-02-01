/* C++ code produced by gperf version 3.1 */
/* Command-line: 'C:\\dev\\geodesk-py\\tools\\gperf' -L C++ -t --class-name=BuildSettings_AttrHash --lookup-function-name=lookup BuildSettings_attr.txt  */
/* Computed positions: -k'6' */

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

#line 1 "BuildSettings_attr.txt"

// Keywords
#include <assert.h>
#include "build/util/BuildSettings.h"

#line 7 "BuildSettings_attr.txt"
struct Attribute { const char *name; int attr; };

#define TOTAL_KEYWORDS 16
#define MIN_WORD_LENGTH 6
#define MAX_WORD_LENGTH 22
#define MIN_HASH_VALUE 6
#define MAX_HASH_VALUE 31
/* maximum key range = 26, duplicates = 0 */

class BuildSettings_AttrHash
{
private:
  static inline unsigned int hash (const char *str, size_t len);
public:
  static struct Attribute *lookup (const char *str, size_t len);
};

inline unsigned int
BuildSettings_AttrHash::hash (const char *str, size_t len)
{
  static unsigned char asso_values[] =
    {
      32, 32, 32, 32, 32, 32, 32, 32, 32, 32,
      32, 32, 32, 32, 32, 32, 32, 32, 32, 32,
      32, 32, 32, 32, 32, 32, 32, 32, 32, 32,
      32, 32, 32, 32, 32, 32, 32, 32, 32, 32,
      32, 32, 32, 32, 32, 32, 32, 32, 32, 32,
      32, 32, 32, 32, 32, 32, 32, 32, 32, 32,
      32, 32, 32, 32, 32, 32, 32, 32, 32, 32,
      32, 32, 32, 32, 32, 32, 32, 32, 32, 32,
      32, 32, 32, 32, 32, 32, 32, 32, 32, 32,
      32, 32, 32, 32, 32, 10, 32,  5, 32, 32,
      10,  0, 32, 32, 32, 10, 32, 32, 20, 32,
       0, 32, 32, 32,  0, 32,  0, 32, 32, 32,
      32, 32, 32, 32, 32, 32, 32, 32, 32, 32,
      32, 32, 32, 32, 32, 32, 32, 32, 32, 32,
      32, 32, 32, 32, 32, 32, 32, 32, 32, 32,
      32, 32, 32, 32, 32, 32, 32, 32, 32, 32,
      32, 32, 32, 32, 32, 32, 32, 32, 32, 32,
      32, 32, 32, 32, 32, 32, 32, 32, 32, 32,
      32, 32, 32, 32, 32, 32, 32, 32, 32, 32,
      32, 32, 32, 32, 32, 32, 32, 32, 32, 32,
      32, 32, 32, 32, 32, 32, 32, 32, 32, 32,
      32, 32, 32, 32, 32, 32, 32, 32, 32, 32,
      32, 32, 32, 32, 32, 32, 32, 32, 32, 32,
      32, 32, 32, 32, 32, 32, 32, 32, 32, 32,
      32, 32, 32, 32, 32, 32, 32, 32, 32, 32,
      32, 32, 32, 32, 32, 32
    };
  return len + asso_values[static_cast<unsigned char>(str[5])];
}

struct Attribute *
BuildSettings_AttrHash::lookup (const char *str, size_t len)
{
  static struct Attribute wordlist[] =
    {
      {""}, {""}, {""}, {""}, {""}, {""},
#line 21 "BuildSettings_attr.txt"
      {"source"},
      {""}, {""},
#line 9 "BuildSettings_attr.txt"
      {"area_tags"},
#line 19 "BuildSettings_attr.txt"
      {"properties"},
#line 15 "BuildSettings_attr.txt"
      {"max_strings"},
#line 12 "BuildSettings_attr.txt"
      {"indexed_keys"},
      {""},
#line 23 "BuildSettings_attr.txt"
      {"updatable"},
#line 14 "BuildSettings_attr.txt"
      {"max_key_indexes"},
#line 17 "BuildSettings_attr.txt"
      {"min_string_usage"},
#line 22 "BuildSettings_attr.txt"
      {"threads"},
      {""},
#line 16 "BuildSettings_attr.txt"
      {"max_tiles"},
      {""},
#line 11 "BuildSettings_attr.txt"
      {"id_indexing"},
#line 13 "BuildSettings_attr.txt"
      {"key_index_min_features"},
#line 10 "BuildSettings_attr.txt"
      {"excluded_keys"},
      {""}, {""},
#line 18 "BuildSettings_attr.txt"
      {"min_tile_density"},
#line 20 "BuildSettings_attr.txt"
      {"rtree_branch_size"},
      {""}, {""}, {""},
#line 24 "BuildSettings_attr.txt"
      {"zoom_levels"}
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
