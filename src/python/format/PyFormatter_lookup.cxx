/* C++ code produced by gperf version 3.1 */
/* Command-line: 'C:\\dev\\geodesk-py\\tools\\gperf' -L C++ -t --class-name=PyFormatter_AttrHash --lookup-function-name=lookup PyFormatter_attr.txt  */
/* Computed positions: -k'1' */

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

#line 1 "PyFormatter_attr.txt"

// Keywords
#include <assert.h>

#line 6 "PyFormatter_attr.txt"
struct Attr { const char *name; int index; };

#define TOTAL_KEYWORDS 14
#define MIN_WORD_LENGTH 2
#define MAX_WORD_LENGTH 12
#define MIN_HASH_VALUE 2
#define MAX_HASH_VALUE 24
/* maximum key range = 23, duplicates = 0 */

class PyFormatter_AttrHash
{
private:
  static inline unsigned int hash (const char *str, size_t len);
public:
  static struct Attr *lookup (const char *str, size_t len);
};

inline unsigned int
PyFormatter_AttrHash::hash (const char *str, size_t len)
{
  static unsigned char asso_values[] =
    {
      25, 25, 25, 25, 25, 25, 25, 25, 25, 25,
      25, 25, 25, 25, 25, 25, 25, 25, 25, 25,
      25, 25, 25, 25, 25, 25, 25, 25, 25, 25,
      25, 25, 25, 25, 25, 25, 25, 25, 25, 25,
      25, 25, 25, 25, 25, 25, 25, 25, 25, 25,
      25, 25, 25, 25, 25, 25, 25, 25, 25, 25,
      25, 25, 25, 25, 25, 25, 25, 25, 25, 25,
      25, 25, 25, 25, 25, 25, 25, 25, 25, 25,
      25, 25, 25, 25, 25, 25, 25, 25, 25, 25,
      25, 25, 25, 25, 25, 25, 25, 25, 25, 25,
      25,  0, 25, 25, 25,  0, 25,  3,  5, 10,
      25, 25,  5, 25, 25,  0, 15, 25, 25, 25,
      25, 25, 25, 25, 25, 25, 25, 25, 25, 25,
      25, 25, 25, 25, 25, 25, 25, 25, 25, 25,
      25, 25, 25, 25, 25, 25, 25, 25, 25, 25,
      25, 25, 25, 25, 25, 25, 25, 25, 25, 25,
      25, 25, 25, 25, 25, 25, 25, 25, 25, 25,
      25, 25, 25, 25, 25, 25, 25, 25, 25, 25,
      25, 25, 25, 25, 25, 25, 25, 25, 25, 25,
      25, 25, 25, 25, 25, 25, 25, 25, 25, 25,
      25, 25, 25, 25, 25, 25, 25, 25, 25, 25,
      25, 25, 25, 25, 25, 25, 25, 25, 25, 25,
      25, 25, 25, 25, 25, 25, 25, 25, 25, 25,
      25, 25, 25, 25, 25, 25, 25, 25, 25, 25,
      25, 25, 25, 25, 25, 25, 25, 25, 25, 25,
      25, 25, 25, 25, 25, 25
    };
  return len + asso_values[static_cast<unsigned char>(str[0])];
}

struct Attr *
PyFormatter_AttrHash::lookup (const char *str, size_t len)
{
  static struct Attr wordlist[] =
    {
      {""}, {""},
#line 9 "PyFormatter_attr.txt"
      {"id", PyFormatter::ID},
      {""},
#line 17 "PyFormatter_attr.txt"
      {"save", PyFormatter::SAVE},
#line 18 "PyFormatter_attr.txt"
      {"scale", PyFormatter::SCALE},
      {""},
#line 10 "PyFormatter_attr.txt"
      {"keys", PyFormatter::KEYS},
#line 19 "PyFormatter_attr.txt"
      {"simplify", PyFormatter::SIMPLIFY},
#line 20 "PyFormatter_attr.txt"
      {"sort_tags", PyFormatter::SORT_TAGS},
#line 11 "PyFormatter_attr.txt"
      {"limit", PyFormatter::LIMIT},
#line 16 "PyFormatter_attr.txt"
      {"pretty", PyFormatter::PRETTY},
#line 8 "PyFormatter_attr.txt"
      {"exclude_keys", PyFormatter::EXCLUDE_KEYS},
#line 12 "PyFormatter_attr.txt"
      {"linewise", PyFormatter::LINEWISE},
#line 15 "PyFormatter_attr.txt"
      {"precision", PyFormatter::PRECISION},
      {""}, {""}, {""},
#line 14 "PyFormatter_attr.txt"
      {"mercator", PyFormatter::MERCATOR},
#line 13 "PyFormatter_attr.txt"
      {"max_width", PyFormatter::MAX_WIDTH},
      {""}, {""}, {""}, {""},
#line 21 "PyFormatter_attr.txt"
      {"translate", PyFormatter::TRANSLATE}
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
