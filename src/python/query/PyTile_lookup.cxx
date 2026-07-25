/* C++ code produced by gperf version 3.1 */
/* Command-line: 'c:\\dev\\geodesk-py\\tools\\gperf' -L C++ -t --class-name=PyTile_AttrHash --lookup-function-name=lookup PyTile_attr.txt  */
/* Computed positions: -k'1,4' */

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

#line 1 "PyTile_attr.txt"

// Keywords
#include <assert.h>
#include <cstdint>
#include "python/util/util.h"

#line 8 "PyTile_attr.txt"
struct PyTileAttribute { const char *name; Python::AttrRef attr; };

#define TOTAL_KEYWORDS 18
#define MIN_WORD_LENGTH 2
#define MAX_WORD_LENGTH 10
#define MIN_HASH_VALUE 2
#define MAX_HASH_VALUE 28
/* maximum key range = 27, duplicates = 0 */

class PyTile_AttrHash
{
private:
  static inline unsigned int hash (const char *str, size_t len);
public:
  static struct PyTileAttribute *lookup (const char *str, size_t len);
};

inline unsigned int
PyTile_AttrHash::hash (const char *str, size_t len)
{
  static unsigned char asso_values[] =
    {
      29, 29, 29, 29, 29, 29, 29, 29, 29, 29,
      29, 29, 29, 29, 29, 29, 29, 29, 29, 29,
      29, 29, 29, 29, 29, 29, 29, 29, 29, 29,
      29, 29, 29, 29, 29, 29, 29, 29, 29, 29,
      29, 29, 29, 29, 29, 29, 29, 29, 29, 29,
      29, 29, 29, 29, 29, 29, 29, 29, 29, 29,
      29, 29, 29, 29, 29, 29, 29, 29, 29, 29,
      29, 29, 29, 29, 29, 29, 29, 29, 29, 29,
      29, 29, 29, 29, 29, 29, 29, 29, 29, 29,
      29, 29, 29, 29, 29, 29, 29,  5,  5,  0,
      29,  0,  0, 29, 29,  0, 29, 29, 10,  5,
       5,  5,  0, 29,  5,  0, 20,  5, 29, 29,
      29, 29,  0, 29, 29, 29, 29, 29, 29, 29,
      29, 29, 29, 29, 29, 29, 29, 29, 29, 29,
      29, 29, 29, 29, 29, 29, 29, 29, 29, 29,
      29, 29, 29, 29, 29, 29, 29, 29, 29, 29,
      29, 29, 29, 29, 29, 29, 29, 29, 29, 29,
      29, 29, 29, 29, 29, 29, 29, 29, 29, 29,
      29, 29, 29, 29, 29, 29, 29, 29, 29, 29,
      29, 29, 29, 29, 29, 29, 29, 29, 29, 29,
      29, 29, 29, 29, 29, 29, 29, 29, 29, 29,
      29, 29, 29, 29, 29, 29, 29, 29, 29, 29,
      29, 29, 29, 29, 29, 29, 29, 29, 29, 29,
      29, 29, 29, 29, 29, 29, 29, 29, 29, 29,
      29, 29, 29, 29, 29, 29, 29, 29, 29, 29,
      29, 29, 29, 29, 29, 29
    };
  unsigned int hval = len;

  switch (hval)
    {
      default:
        hval += asso_values[static_cast<unsigned char>(str[3])];
      /*FALLTHROUGH*/
      case 3:
      case 2:
      case 1:
        hval += asso_values[static_cast<unsigned char>(str[0])];
        break;
    }
  return hval;
}

struct PyTileAttribute *
PyTile_AttrHash::lookup (const char *str, size_t len)
{
  static struct PyTileAttribute wordlist[] =
    {
      {""}, {""},
#line 16 "PyTile_attr.txt"
      {"id", ATTR_PROPERTY(PyTile::id)},
#line 12 "PyTile_attr.txt"
      {"col", ATTR_PROPERTY(PyTile::column)},
#line 25 "PyTile_attr.txt"
      {"size", ATTR_PROPERTY(PyTile::size)},
#line 24 "PyTile_attr.txt"
      {"shape", ATTR_PROPERTY(PyTile::shape)},
#line 21 "PyTile_attr.txt"
      {"parent", ATTR_PROPERTY(PyTile::parent)},
#line 17 "PyTile_attr.txt"
      {"indexes", ATTR_PROPERTY(PyTile::indexes)},
#line 23 "PyTile_attr.txt"
      {"row", ATTR_PROPERTY(PyTile::row)},
#line 27 "PyTile_attr.txt"
      {"zoom", ATTR_PROPERTY(PyTile::zoom)},
#line 19 "PyTile_attr.txt"
      {"is_current", ATTR_PROPERTY(PyTile::id)},
#line 13 "PyTile_attr.txt"
      {"column", ATTR_PROPERTY(PyTile::column)},
#line 14 "PyTile_attr.txt"
      {"exports", ATTR_PROPERTY(PyTile::exports)},
#line 22 "PyTile_attr.txt"
      {"revision", ATTR_PROPERTY(PyTile::revision)},
#line 18 "PyTile_attr.txt"
      {"is_active", ATTR_PROPERTY(PyTile::id)},
      {""},
#line 10 "PyTile_attr.txt"
      {"bounds", ATTR_PROPERTY(PyTile::bounds)},
      {""},
#line 11 "PyTile_attr.txt"
      {"children", ATTR_PROPERTY(PyTile::children)},
#line 20 "PyTile_attr.txt"
      {"is_loaded", ATTR_PROPERTY(PyTile::id)},
      {""}, {""}, {""},
#line 26 "PyTile_attr.txt"
      {"tip", ATTR_PROPERTY(PyTile::tip)},
      {""}, {""}, {""}, {""},
#line 15 "PyTile_attr.txt"
      {"features", ATTR_PROPERTY(PyTile::features)}
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
