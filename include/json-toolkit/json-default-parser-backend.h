// Copyright (C) 2019 Vincent Chambrin
// This file is part of the json-toolkit library
// For conditions of distribution and use, see copyright notice in LICENSE

namespace json
{

struct DefaultTokenizerBackend
{
  std::vector<json::Token> token_buffer;

  typedef std::string string_type;
  typedef char char_type;

  static json::CharCategory category(char_type c)
  {
    using namespace json;

    switch (c)
    {
    case ' ': return CharCategory::Space;
    case '\n': return CharCategory::NewLine;
    case 'e': return CharCategory::ExponentSymbol;
    case '\'': return CharCategory::SingleQuote;
    case '"': return CharCategory::DoubleQuote;
    case '.': return CharCategory::Dot;
    case ',': return CharCategory::Comma;
    case ':': return CharCategory::Colon;
    case '{': return CharCategory::LBrace;
    case '}': return CharCategory::RBrace;
    case '[': return CharCategory::LBracket;
    case ']': return CharCategory::RBracket;
    case '+': return CharCategory::PlusSign;
    case '-': return CharCategory::MinusSign;
    case '_': return CharCategory::Underscore;
    default:
      break;
    }

    if ('0' <= c && c <= '9')
      return CharCategory::Digit;
    else if ('a' <= c && c <= 'z' || 'A' <= c && c <= 'Z')
      return CharCategory::Letter;

    if ('!' <= c && c <= '/')
      return CharCategory::Other;

    return CharCategory::Invalid;
  }

  static bool is_null(const string_type& str)
  {
    return str == "null";
  }

  static bool is_bool(const string_type& str, bool* value)
  {
    if (str == "true")
    {
      *value = true;
      return true;
    }
    else if (str == "false")
    {
      *value = false;
      return true;
    }

    return false;
  }

  static char_type new_line() { return '\n'; }

  static size_t size(const string_type& str)
  {
    return str.size();
  }

  static char_type at(const string_type& str, size_t index)
  {
    return str.at(index);
  }

  static void clear(string_type& str)
  {
    str.clear();
  }

  static void push_back(string_type& str, char_type c)
  {
    str.push_back(c);
  }

  void produce(json::TokenType ttype, const string_type& str)
  {
    json::Token tok{ ttype, str };
    token_buffer.push_back(tok);
  }
};

struct DefaultParserBackend
{
  static int parse_integer(const std::string& str)
  {
    return std::stoi(str);
  }

  static double parse_number(const std::string& str)
  {
    return std::stod(str);
  }

  static std::string remove_quotes(const std::string& str)
  {
    return std::string(str.begin() + 1, str.end() - 1);
  }

  void writeField(const json::Json& value)
  {
    assert(stack.back().isString());

    std::string key = stack.back().toString();
    stack.pop_back();

    assert(stack.back().isObject());

    stack.back()[key] = value;
  }

  void writeValue(const json::Json& value)
  {
    if (stack.back().isString())
    {
      writeField(value);
    }
    else
    {
      assert(stack.back().isArray());
      stack.back().push(value);
    }
  }

  void value(std::nullptr_t)
  {
    writeValue(json::Json(nullptr));
  }

  void value(bool val)
  {
    writeValue(json::Json(val));
  }

  void value(int val)
  {
    writeValue(json::Json(val));
  }

  void value(double val)
  {
    writeValue(json::Json(val));
  }

  void value(const std::string& str)
  {
    writeValue(json::Json(str));
  }

  void start_object()
  {
    stack.push_back(json::Object());
  }

  void key(const std::string& str)
  {
    assert(stack.back().isObject());
    stack.push_back(json::Json(str));
  }

  void end_object()
  {
    if (stack.size() == 1)
      return;

    auto object = stack.back();
    stack.pop_back();

    if (stack.back().isString())
    {
      std::string key = stack.back().toString();
      stack.pop_back();

      assert(stack.back().isObject());

      stack.back()[key] = object;

    }
    else if (stack.back().isArray())
    {
      json::Array a = stack.back().toArray();
      a.push(object);
    }
  }

  void start_array()
  {
    stack.push_back(json::Array());
  }

  void end_array()
  {
    if (stack.size() == 1)
      return;

    auto vec = stack.back();
    stack.pop_back();

    if (stack.back().isString())
    {
      std::string key = stack.back().toString();
      stack.pop_back();

      assert(stack.back().isObject());

      stack.back()[key] = vec;

    }
    else if (stack.back().isArray())
    {
      json::Array a = stack.back().toArray();
      a.push(vec);
    }
  }

  std::vector<json::Json> stack;
};

} // namespace json
