// Copyright (C) 2019 Vincent Chambrin
// This file is part of the json-toolkit library
// For conditions of distribution and use, see copyright notice in LICENSE

#ifndef JSONTOOLKIT_GLOBAL_DEFS_H
#define JSONTOOLKIT_GLOBAL_DEFS_H

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

#endif // !JSONTOOLKIT_GLOBAL_DEFS_H