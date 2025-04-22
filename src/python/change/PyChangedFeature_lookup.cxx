/* C++ code produced by gperf version 3.1 */
/* Command-line: 'C:\\dev\\geodesk-py\\tools\\gperf' -L C++ -t --class-name=PyChangedFeature_AttrHash --lookup-function-name=lookup PyChangedFeature_attr.txt  */
/* Computed positions: -k'2,$' */

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

#line 1 "PyChangedFeature_attr.txt"

// Keywords
#include "PyChangedFeature.h"

#line 6 "PyChangedFeature_attr.txt"
static struct Attribute { const char *name; PyChangedFeature::Attr index; };

#define TOTAL_KEYWORDS 21
#define MIN_WORD_LENGTH 1
#define MAX_WORD_LENGTH 11
#define MIN_HASH_VALUE 1
#define MAX_HASH_VALUE 36
/* maximum key range = 36, duplicates = 0 */

class PyChangedFeature_AttrHash
{
private:
  static inline unsigned int hash (const char *str, size_t len);
public:
  static static struct Attribute *lookup (const char *str, size_t len);
};

inline unsigned int
PyChangedFeature_AttrHash::hash (const char *str, size_t len)
{
  static unsigned char asso_values[] =
    {
      37, 37, 37, 37, 37, 37, 37, 37, 37, 37,
      37, 37, 37, 37, 37, 37, 37, 37, 37, 37,
      37, 37, 37, 37, 37, 37, 37, 37, 37, 37,
      37, 37, 37, 37, 37, 37, 37, 37, 37, 37,
      37, 37, 37, 37, 37, 37, 37, 37, 37, 37,
      37, 37, 37, 37, 37, 37, 37, 37, 37, 37,
      37, 37, 37, 37, 37, 37, 37, 37, 37, 37,
      37, 37, 37, 37, 37, 37, 37, 37, 37, 37,
      37, 37, 37, 37, 37, 37, 37, 37, 37, 37,
      37, 37, 37, 37, 37, 37, 37, 37,  0, 37,
      20, 10, 20, 37, 37,  0, 37, 37,  0, 37,
       5, 37,  5,  0, 37,  0,  0, 37, 37, 37,
      20,  0, 37, 37, 37, 37, 37, 37, 37, 37,
      37, 37, 37, 37, 37, 37, 37, 37, 37, 37,
      37, 37, 37, 37, 37, 37, 37, 37, 37, 37,
      37, 37, 37, 37, 37, 37, 37, 37, 37, 37,
      37, 37, 37, 37, 37, 37, 37, 37, 37, 37,
      37, 37, 37, 37, 37, 37, 37, 37, 37, 37,
      37, 37, 37, 37, 37, 37, 37, 37, 37, 37,
      37, 37, 37, 37, 37, 37, 37, 37, 37, 37,
      37, 37, 37, 37, 37, 37, 37, 37, 37, 37,
      37, 37, 37, 37, 37, 37, 37, 37, 37, 37,
      37, 37, 37, 37, 37, 37, 37, 37, 37, 37,
      37, 37, 37, 37, 37, 37, 37, 37, 37, 37,
      37, 37, 37, 37, 37, 37, 37, 37, 37, 37,
      37, 37, 37, 37, 37, 37, 37
    };
  unsigned int hval = len;

  switch (hval)
    {
      default:
        hval += asso_values[static_cast<unsigned char>(str[1]+1)];
      /*FALLTHROUGH*/
      case 1:
        break;
    }
  return hval + asso_values[static_cast<unsigned char>(str[len - 1])];
}

static struct Attribute *
PyChangedFeature_AttrHash::lookup (const char *str, size_t len)
{
  static static struct Attribute wordlist[] =
    {
      {""},
#line 16 "PyChangedFeature_attr.txt"
      {"y", PyChangedFeature::Attr::Y},
      {""},
#line 8 "PyChangedFeature_attr.txt"
      {"lat", PyChangedFeature::Attr::LAT},
#line 14 "PyChangedFeature_attr.txt"
      {"tags", PyChangedFeature::Attr::TAGS},
#line 28 "PyChangedFeature_attr.txt"
      {"split", PyChangedFeature::Attr::SPLIT},
#line 24 "PyChangedFeature_attr.txt"
      {"is_way", PyChangedFeature::Attr::IS_WAY},
      {""},
#line 26 "PyChangedFeature_attr.txt"
      {"original", PyChangedFeature::Attr::ORIGINAL},
      {""},
#line 11 "PyChangedFeature_attr.txt"
      {"nodes", PyChangedFeature::Attr::NODES},
#line 25 "PyChangedFeature_attr.txt"
      {"modify", PyChangedFeature::Attr::MODIFY},
#line 18 "PyChangedFeature_attr.txt"
      {"connect", PyChangedFeature::Attr::CONNECT},
#line 9 "PyChangedFeature_attr.txt"
      {"lon", PyChangedFeature::Attr::LON},
      {""},
#line 13 "PyChangedFeature_attr.txt"
      {"shape", PyChangedFeature::Attr::SHAPE},
#line 23 "PyChangedFeature_attr.txt"
      {"is_relation", PyChangedFeature::Attr::IS_RELATION},
#line 22 "PyChangedFeature_attr.txt"
      {"is_node", PyChangedFeature::Attr::IS_NODE},
#line 27 "PyChangedFeature_attr.txt"
      {"osm_type", PyChangedFeature::Attr::OSM_TYPE},
#line 12 "PyChangedFeature_attr.txt"
      {"role", PyChangedFeature::Attr::ROLE},
      {""},
#line 15 "PyChangedFeature_attr.txt"
      {"x", PyChangedFeature::Attr::X},
#line 17 "PyChangedFeature_attr.txt"
      {"combine", PyChangedFeature::Attr::COMBINE},
      {""}, {""}, {""}, {""},
#line 10 "PyChangedFeature_attr.txt"
      {"members", PyChangedFeature::Attr::MEMBERS},
      {""}, {""},
#line 21 "PyChangedFeature_attr.txt"
      {"is_deleted", PyChangedFeature::Attr::IS_DELETED},
      {""},
#line 20 "PyChangedFeature_attr.txt"
      {"id", PyChangedFeature::Attr::ID},
      {""}, {""}, {""},
#line 19 "PyChangedFeature_attr.txt"
      {"delete", PyChangedFeature::Attr::DELETE}
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
