
#ifndef LIBJSON_PARSING_H
#define LIBJSON_PARSING_H

#include "json.h"

namespace json
{

enum class TokenType {
  Invalid = 0,
  Identifier,
  LBrace,
  RBrace,
  LBracket,
  RBracket,
  Colon,
  Comma,
  True,
  False,
  Number,
  StringLiteral,
};

class Token
{
public:
  TokenType type;
  config::string_type text;

public:
  Token() : type(TokenType::Invalid) { }
  Token(const Token&) = default;
  ~Token() = default;

  Token(TokenType ttype, const config::string_type& str = config::string_type())
    : type(ttype), text(str) { }

  Token& operator=(const Token&) = default;
};

inline bool operator==(const Token& lhs, const Token& rhs)
{
  return lhs.type == rhs.type && lhs.text == rhs.text;
}

inline bool operator!=(const Token& lhs, const Token& rhs)
{
  return !(lhs == rhs);
}

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

/*
struct TokenizerBackend
{
  typedef std::string string_type;
  typedef char char_type;

  static CharCategory category(char_type c);
  static bool is_bool(const string_type& str, bool* value);
  static char_type new_line();

  static size_t size(const string_type& str);
  static char_type at(const string_type& str, size_t index);
  static void clear(string_type& str);
  static void push_back(string_type& str, char_type c);

  void produce(TokenType ttype, const string_type& str);
};
*/

enum class TokenizerState {
  Idle = 0,
  ParsingIdentifier,
  ParsingNumberSign,
  ParsingNumber,
  ParsingDecimals,
  ParsedExponentSymbol,
  ParsingExponentSign,
  ParsingExponent,
  ParsingSingleQuoteString,
  ParsingDoubleQuoteString,
};

template<typename Backend>
class Tokenizer
{
public:
  Tokenizer()
    : m_state(TokenizerState::Idle)
  {

  }

  ~Tokenizer() = default;

  using String = typename Backend::string_type;
  using Char = typename Backend::char_type;

  inline TokenizerState state() const { return m_state; }
  inline Backend& backend() { return m_backend; }
  inline String& buffer() { return m_buffer; }

  void write(Char c)
  {
    CharCategory cc = m_backend.category(c);

    if (cc == CharCategory::Invalid)
      throw std::runtime_error{ "Invalid input" };

    switch (m_state)
    {
    case TokenizerState::Idle: return StateIdle(c, cc);
    case TokenizerState::ParsingIdentifier: return StateParsingIdentifier(c, cc);
    case TokenizerState::ParsingNumberSign: return StateParsingNumberSign(c, cc);
    case TokenizerState::ParsingNumber: return StateParsingNumber(c, cc);
    case TokenizerState::ParsingDecimals: return StateParsingDecimals(c, cc);
    case TokenizerState::ParsedExponentSymbol: return StateParsedExponentSymbol(c, cc);
    case TokenizerState::ParsingExponentSign: return StateParsingExponentSign(c, cc);
    case TokenizerState::ParsingExponent: return StateParsingExponent(c, cc);
    case TokenizerState::ParsingSingleQuoteString: return StateParsingSingleQuoteString(c, cc);
    case TokenizerState::ParsingDoubleQuoteString: return StateParsingDoubleQuoteString(c, cc);
    }
  }

  void write(const String& str)
  {
    auto s = m_backend.size(str);
    for (decltype(s) i = 0; i < s; ++i)
      write(m_backend.at(str, i));
  }

  void done()
  {
    write(m_backend.new_line());
  }

protected:
  void produce(TokenType t)
  {
    m_backend.produce(t, m_buffer);
    m_buffer.clear();
  }

  void produceIdentifier()
  {
    bool value;

    if (m_backend.is_bool(m_buffer, &value))
      produce(value ? TokenType::True : TokenType::False);
    else
      produce(TokenType::Identifier);
  }

  void push(Char c)
  {
    m_backend.push_back(m_buffer, c);
  }

  void enter(TokenizerState s)
  {
    m_state = s;
  }

  void StateIdle(Char c, CharCategory cc)
  {
    switch (cc)
    {
    case CharCategory::Space:
    case CharCategory::NewLine:
      return;
    case CharCategory::LBrace:
      return produce(TokenType::LBrace);
    case CharCategory::RBrace:
      return produce(TokenType::RBrace);
    case CharCategory::LBracket:
      return produce(TokenType::LBracket);
    case CharCategory::RBracket:
      return produce(TokenType::RBracket);
    case CharCategory::Colon:
      return produce(TokenType::Colon);
    case CharCategory::Comma:
      return produce(TokenType::Comma);
    case CharCategory::Underscore:
    case CharCategory::Letter:
    case CharCategory::ExponentSymbol:
      enter(TokenizerState::ParsingIdentifier);
      push(c);
      return;
    case CharCategory::PlusSign:
    case CharCategory::MinusSign:
      enter(TokenizerState::ParsingNumberSign);
      push(c);
      return;
    case CharCategory::Digit:
      enter(TokenizerState::ParsingNumber);
      push(c);
      return;
    case CharCategory::SingleQuote:
      enter(TokenizerState::ParsingSingleQuoteString);
      push(c);
      return;
    case CharCategory::DoubleQuote:
      enter(TokenizerState::ParsingDoubleQuoteString);
      push(c);
      break;
    case CharCategory::Dot:
    case CharCategory::Other:
    default:
      throw std::runtime_error{ "Invalid input in 'Idle' state" };
      break;
    }
  }

  void StateParsingIdentifier(Char c, CharCategory cc)
  {
    switch (cc)
    {
    case CharCategory::Underscore:
    case CharCategory::Letter:
    case CharCategory::ExponentSymbol:
    case CharCategory::Digit:
      return push(c);
    case CharCategory::Space:
    case CharCategory::NewLine:
    case CharCategory::LBrace:
    case CharCategory::RBrace:
    case CharCategory::LBracket:
    case CharCategory::RBracket:
    case CharCategory::Colon:
    case CharCategory::Comma:
    case CharCategory::PlusSign:
    case CharCategory::MinusSign:
    case CharCategory::SingleQuote:
    case CharCategory::DoubleQuote:
      produceIdentifier();
      enter(TokenizerState::Idle);
      return StateIdle(c, cc);
    case CharCategory::Other:
    default:
      throw std::runtime_error{ "Invalid input in 'ParsingIdentifier' state" };
      break;
    }
  }

  void StateParsingNumberSign(Char c, CharCategory cc)
  {
    switch (cc)
    {
    case CharCategory::PlusSign:
    case CharCategory::MinusSign:
      return push(c);
    case CharCategory::Digit:
      push(c);
      return enter(TokenizerState::ParsingNumber);
    default:
      throw std::runtime_error{ "Invalid input in 'ParsingNumberSign' state" };
      break;
    }
  }

  void StateParsingNumber(Char c, CharCategory cc)
  {
    switch (cc)
    {
    case CharCategory::Digit:
      push(c);
      return;
    case CharCategory::Dot:
      push(c);
      enter(TokenizerState::ParsingDecimals);
      return;
    case CharCategory::ExponentSymbol:
      push(c);
      enter(TokenizerState::ParsedExponentSymbol);
      return;
    case CharCategory::Space:
    case CharCategory::NewLine:
    case CharCategory::LBrace:
    case CharCategory::RBrace:
    case CharCategory::LBracket:
    case CharCategory::RBracket:
    case CharCategory::Colon:
    case CharCategory::Comma:
    case CharCategory::SingleQuote:
    case CharCategory::DoubleQuote:
      produce(TokenType::Number);
      enter(TokenizerState::Idle);
      StateIdle(c, cc);
      return;
    default:
      throw std::runtime_error{ "Invalid input in 'ParsingNumber' state" };
      break;
    }
  }

  void StateParsingDecimals(Char c, CharCategory cc)
  {
    switch (cc)
    {
    case CharCategory::Digit:
      push(c);
      return;
    case CharCategory::Dot:
      throw std::runtime_error{ "Invalid input '.' in 'ParsingDecimals' state" };
    case CharCategory::ExponentSymbol:
      push(c);
      enter(TokenizerState::ParsedExponentSymbol);
      return;
    case CharCategory::Space:
    case CharCategory::NewLine:
    case CharCategory::LBrace:
    case CharCategory::RBrace:
    case CharCategory::LBracket:
    case CharCategory::RBracket:
    case CharCategory::Colon:
    case CharCategory::Comma:
    case CharCategory::SingleQuote:
    case CharCategory::DoubleQuote:
      produce(TokenType::Number);
      enter(TokenizerState::Idle);
      StateIdle(c, cc);
      return;
    default:
      throw std::runtime_error{ "Invalid input in 'ParsingDecimals' state" };
      break;
    }
  }

  void StateParsedExponentSymbol(Char c, CharCategory cc)
  {
    switch (cc)
    {
    case CharCategory::Digit:
      push(c);
      enter(TokenizerState::ParsingExponent);
      return;
    case CharCategory::Dot:
      throw std::runtime_error{ "Invalid input '.' in 'ParsedExponentSymbol' state" };
    case CharCategory::ExponentSymbol:
      throw std::runtime_error{ "Invalid input 'e' in 'ParsedExponentSymbol' state" };
    case CharCategory::PlusSign:
    case CharCategory::MinusSign:
      push(c);
      enter(TokenizerState::ParsingExponentSign);
      return;
    default:
      throw std::runtime_error{ "Invalid input in 'ParsedExponentSymbol' state" };
      break;
    }
  }

  void StateParsingExponentSign(Char c, CharCategory cc)
  {
    switch (cc)
    {
    case CharCategory::Digit:
      enter(TokenizerState::ParsingExponent);
      push(c);
      return;
    case CharCategory::PlusSign:
    case CharCategory::MinusSign:
      push(c);
      return;
    case CharCategory::ExponentSymbol:
      throw std::runtime_error{ "Invalid input 'e' in 'ParsingExponentSign' state" };
    case CharCategory::Dot:
      throw std::runtime_error{ "Invalid input '.' in 'ParsingExponentSign' state" };
    default:
      throw std::runtime_error{ "Invalid input in 'ParsingExponentSign' state" };
      break;
    }
  }

  void StateParsingExponent(Char c, CharCategory cc)
  {
    switch (cc)
    {
    case CharCategory::Digit:
      push(c);
      return;
    case CharCategory::Dot:
      throw std::runtime_error{ "Invalid input '.' in 'ParsingExponent' state" };
    case CharCategory::ExponentSymbol:
      throw std::runtime_error{ "Invalid input 'e' in 'ParsingExponent' state" };
    case CharCategory::Space:
    case CharCategory::NewLine:
    case CharCategory::LBrace:
    case CharCategory::RBrace:
    case CharCategory::LBracket:
    case CharCategory::RBracket:
    case CharCategory::Colon:
    case CharCategory::Comma:
    case CharCategory::SingleQuote:
    case CharCategory::DoubleQuote:
      produce(TokenType::Number);
      enter(TokenizerState::Idle);
      StateIdle(c, cc);
      return;
    default:
      throw std::runtime_error{ "Invalid input in 'ParsingExponent' state" };
      break;
    }
  }

  void StateParsingSingleQuoteString(Char c, CharCategory cc)
  {
    switch (cc)
    {
    case CharCategory::SingleQuote:
      push(c);
      produce(TokenType::StringLiteral);
      enter(TokenizerState::Idle);
      return;
    case CharCategory::NewLine:
      throw std::runtime_error{ "Invalid input 'NewLine' in 'ParsingSingleQuoteString' state" };
    default:
      push(c);
      return;
    }
  }

  void StateParsingDoubleQuoteString(Char c, CharCategory cc)
  {
    switch (cc)
    {
    case CharCategory::DoubleQuote:
      push(c);
      produce(TokenType::StringLiteral);
      enter(TokenizerState::Idle);
      return;
    case CharCategory::NewLine:
      throw std::runtime_error{ "Invalid input 'NewLine' in 'ParsingDoubleQuoteString' state" };
    default:
      push(c);
      return;
    }
  }

private:
  Backend m_backend;
  String m_buffer;
  TokenizerState m_state;
};

} // namespace json


#endif // !LIBJSON_PARSING_H