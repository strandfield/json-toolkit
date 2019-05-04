
#include <gtest/gtest.h>

#include "parsing.h"

struct TokenizerBackend
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

    return CharCategory::Invalid;
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

TEST(parsing, tokenizer)
{
  using namespace json;
  
  Tokenizer<TokenizerBackend> tokenizer;
  auto& buffer = tokenizer.backend().token_buffer;

  tokenizer.write("123 hello 'str' \"haha\" ");

  ASSERT_EQ(buffer.size(), 4);
  ASSERT_EQ(buffer.front(), Token(TokenType::Number, "123"));
  ASSERT_EQ(buffer.at(1), Token(TokenType::Identifier, "hello"));
  ASSERT_EQ(buffer.at(2), Token(TokenType::StringLiteral, "'str'"));
  ASSERT_EQ(buffer.back(), Token(TokenType::StringLiteral, "\"haha\""));

  buffer.clear();
  tokenizer.write("tru");
  ASSERT_TRUE(buffer.empty());
  ASSERT_EQ(tokenizer.state(), TokenizerState::ParsingIdentifier);

  tokenizer.write('e');
  tokenizer.done();
  ASSERT_EQ(buffer.size(), 1);
  ASSERT_EQ(buffer.front(), Token(TokenType::True, "true"));

  buffer.clear();
  tokenizer.write("[]{},:");
  ASSERT_EQ(buffer.size(), 6);
  ASSERT_EQ(buffer.at(0), Token(TokenType::LBracket, ""));
  ASSERT_EQ(buffer.at(1), Token(TokenType::RBracket, ""));
  ASSERT_EQ(buffer.at(2), Token(TokenType::LBrace, ""));
  ASSERT_EQ(buffer.at(3), Token(TokenType::RBrace, ""));
  ASSERT_EQ(buffer.at(4), Token(TokenType::Comma, ""));
  ASSERT_EQ(buffer.at(5), Token(TokenType::Colon, ""));

  buffer.clear();

  tokenizer.write("125 1.31 1e+28 -2.45e-27 ");
  //tokenizer.write("1e+28 -2.45e-27 ");
  ASSERT_EQ(buffer.size(), 4);
  ASSERT_EQ(buffer.at(0), Token(TokenType::Number, "125"));
  ASSERT_EQ(buffer.at(1), Token(TokenType::Number, "1.31"));
  ASSERT_EQ(buffer.at(2), Token(TokenType::Number, "1e+28"));
  ASSERT_EQ(buffer.at(3), Token(TokenType::Number, "-2.45e-27"));
}

TEST(parsing, tokenizer_exceptions)
{
  using namespace json;

  Tokenizer<TokenizerBackend> tokenizer;
  auto& buffer = tokenizer.backend().token_buffer;

  tokenizer.write("1.24");

  ASSERT_ANY_THROW(tokenizer.write('.'));

  tokenizer.write("27 \"string");
  ASSERT_EQ(buffer.size(), 1);

  ASSERT_ANY_THROW(tokenizer.write('\n'));

  tokenizer.write("\" ");
  ASSERT_EQ(buffer.size(), 2);

  ASSERT_ANY_THROW(tokenizer.write('.'));

  tokenizer.write("1.27e12");

  ASSERT_ANY_THROW(tokenizer.write('e'));

  tokenizer.done();

  ASSERT_EQ(buffer.size(), 3);
}
