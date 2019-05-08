#ifndef LIBJSON_GLOBAL_DEFS_H
#define LIBJSON_GLOBAL_DEFS_H

#include <map>
#include <string>
#include <vector>

namespace json
{

enum class CharCategory {
  Invalid = 0,
  Space,
  NewLine,
  LBrace,
  RBrace,
  LBracket,
  RBracket,
  Colon,
  Comma,
  Dot,
  Underscore,
  Letter,
  Digit,
  PlusSign,
  MinusSign,
  ExponentSymbol,
  SingleQuote,
  DoubleQuote,
  Other,
};

} // namespace json

#endif // !LIBJSON_GLOBAL_DEFS_H