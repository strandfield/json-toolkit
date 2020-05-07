// Copyright (C) 2019 Vincent Chambrin
// This file is part of the json-toolkit library
// For conditions of distribution and use, see copyright notice in LICENSE

#include "json-global-defs.h"

#include <sstream>

namespace json
{

struct DefaultWriterBackend
{
  std::stringstream result_;

  std::string result() { return result_.str(); }

  DefaultWriterBackend& operator<<(CharCategory c)
  {
    switch (c)
    {
    case CharCategory::Space:
      result_ << " ";
      break;
    case CharCategory::NewLine:
      result_ << "\n";
      break;
    case CharCategory::LBrace:
      result_ << "{";
      break;
    case CharCategory::RBrace:
      result_ << "}";
      break;
    case CharCategory::LBracket:
      result_ << "[";
      break;
    case CharCategory::RBracket:
      result_ << "]";
      break;
    case CharCategory::Colon:
      result_ << ":";
      break;
    case CharCategory::Comma:
      result_ << ",";
      break;
    case CharCategory::SingleQuote:
      result_ << "'";
      break;
    case CharCategory::DoubleQuote:
      result_ << "\"";
      break;
    default:
      break;
    }

    return *this;
  }

  DefaultWriterBackend& operator<<(std::nullptr_t)
  {
    result_ << "null";
    return *this;
  }

  DefaultWriterBackend& operator<<(bool value)
  {
    result_ << (value ? "true" : "false");
    return *this;
  }

  DefaultWriterBackend & operator<<(int value)
  {
    result_ << value;
    return *this;
  }

  DefaultWriterBackend& operator<<(double value)
  {
    result_ << value;
    return *this;
  }

  DefaultWriterBackend& operator<<(const std::string& str)
  {
    for (char c : str)
    {
      if (c == '\\')
        result_ << "\\\\";
      else if (c == '\n')
        result_ << "\\n";
      else if (c == '\t')
        result_ << "\\t";
      else
        result_ << c;
    }

    return *this;
  }
};

} // namespace json
