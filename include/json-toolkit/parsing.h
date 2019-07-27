// Copyright (C) 2019 Vincent Chambrin
// This file is part of the json-toolkit library
// For conditions of distribution and use, see copyright notice in LICENSE

#ifndef JSONTOOLKIT_PARSING_H
#define JSONTOOLKIT_PARSING_H

#include "json-toolkit/json.h"

namespace json
{

json::Json parse(const std::string& str);

enum class TokenType {
  Invalid = 0,
  Identifier,
  LBrace,
  RBrace,
  LBracket,
  RBracket,
  Colon,
  Comma,
  Null,
  True,
  False,
  Integer,
  Number,
  StringLiteral,
};

class Token
{
public:
  TokenType type;
  std::string text;

public:
  Token() : type(TokenType::Invalid) { }
  Token(const Token&) = default;
  ~Token() = default;

  Token(TokenType ttype, const std::string& str = std::string())
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

/*
struct TokenizerBackend
{
  typedef std::string string_type;
  typedef char char_type;

  static CharCategory category(char_type c);
  static bool is_null(const string_type& str);
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
    else if (m_backend.is_null(m_buffer))
      produce(TokenType::Null);
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
      produce(TokenType::Integer);
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

namespace json
{
/*
struct ParserBackend
{
  static int parse_integer(const std::string& str);
  static double parse_number(const std::string& str);
  static std::string remove_quotes(const std::string& str);

  void value(std::nullptr_t);
  void value(bool val);
  void value(int val);
  void value(double val);
  void value(const std::string& str);

  void start_object();
  void key(const std::string& str);
  void end_object();

  void start_array();
  void end_array();
};
*/

enum class ParserState {
  Idle = 0,
  ParsingObject,
  ReadFieldName,
  ReadFieldColon,
  ReadFieldValue,
  ParsingArray,
  ReadArrayElement,
  ReadArraySeparator,
};

template<typename Backend>
class ParserMachine
{
public:
  ParserMachine()
  {
    m_states.push_back(ParserState::Idle);
  }

  ~ParserMachine() = default;

  inline ParserState state() const { return m_states.back(); }
  inline const std::vector<ParserState>& stack() const { return m_states; }

  inline Backend& backend() { return m_backend; }
  inline std::vector<Token> & buffer() { return m_buffer; }

  void write(const Token& tok)
  {
    switch (state())
    {
    case ParserState::Idle: return StateIdle(tok);
    case ParserState::ParsingObject: return StateParsingObject(tok);
    case ParserState::ReadFieldName: return StateReadFieldName(tok);
    case ParserState::ReadFieldColon: return StateReadFieldColon(tok);
    case ParserState::ReadFieldValue: return StateReadFieldValue(tok);
    case ParserState::ParsingArray: return StateParsingArray(tok);
    case ParserState::ReadArrayElement: return StateReadArrayElement(tok);
    case ParserState::ReadArraySeparator: return StateReadArraySeparator(tok);
    }
  }

protected:

  void enter(ParserState s)
  {
    m_states.push_back(s);
  }

  void update(ParserState s)
  {
    m_states.back() = s;
  }

  void leave()
  {
    m_states.pop_back();

    if (m_states.back() == ParserState::ReadFieldColon)
      m_states.back() = ParserState::ReadFieldValue;
    else if (m_states.back() == ParserState::ParsingArray)
      m_states.back() = ParserState::ReadArrayElement;
  }

  void StateIdle(const Token& tok)
  {
    switch (tok.type)
    {
    case TokenType::LBrace:
      m_backend.start_object();
      enter(ParserState::ParsingObject);
      return;
    case TokenType::LBracket:
      m_backend.start_array();
      enter(ParserState::ParsingArray);
      return;
    default:
      throw std::runtime_error{ "Invalid input in 'Idle' state" };
      break;
    }
  }

  void StateParsingObject(const Token& tok)
  {
    switch (tok.type)
    {
    case TokenType::Identifier:
    case TokenType::StringLiteral:
      m_backend.key(tok.text);
      update(ParserState::ReadFieldName);
      return;
    case TokenType::RBrace:
      m_backend.end_object();
      leave();
      return;
    default:
      throw std::runtime_error{ "Invalid input in 'ParsingObject' state" };
      break;
    }
  }

  void StateReadFieldName(const Token& tok)
  {
    switch (tok.type)
    {
    case TokenType::Colon:
      update(ParserState::ReadFieldColon);
      return;
    default:
      throw std::runtime_error{ "Invalid input in 'ReadFieldName' state" };
      break;
    }
  }

  void StateReadFieldColon(const Token& tok)
  {
    switch (tok.type)
    {
    case TokenType::LBrace:
      m_backend.start_object();
      enter(ParserState::ParsingObject);
      return;
    case TokenType::LBracket:
      m_backend.start_array();
      enter(ParserState::ParsingArray);
      return;
    case TokenType::Null:
      m_backend.value(nullptr);
      update(ParserState::ReadFieldValue);
      return;
    case TokenType::True:
    case TokenType::False:
      m_backend.value(tok.type == TokenType::True);
      update(ParserState::ReadFieldValue);
      return;
    case TokenType::Integer:
      m_backend.value(m_backend.parse_integer(tok.text));
      update(ParserState::ReadFieldValue);
      return;
    case TokenType::Number:
      m_backend.value(m_backend.parse_number(tok.text));
      update(ParserState::ReadFieldValue);
      return;
    case TokenType::StringLiteral:
      m_backend.value(m_backend.remove_quotes(tok.text));
      update(ParserState::ReadFieldValue);
      return;
    default:
      throw std::runtime_error{ "Invalid input in 'ReadFieldColon' state" };
      break;
    }
  }

  void StateReadFieldValue(const Token& tok)
  {
    switch (tok.type)
    {
    case TokenType::Comma:
      update(ParserState::ParsingObject);
      return;
    case TokenType::RBrace:
      m_backend.end_object();
      leave();
      return;
    default:
      throw std::runtime_error{ "Invalid input in 'ReadFieldValue' state" };
      break;
    }
  }

  void StateParsingArray(const Token& tok)
  {
    switch (tok.type)
    {
    case TokenType::LBrace:
      m_backend.start_object();
      enter(ParserState::ParsingObject);
      return;
    case TokenType::LBracket:
      m_backend.start_array();
      enter(ParserState::ParsingArray);
      return;
    case TokenType::Null:
      m_backend.value(nullptr);
      update(ParserState::ReadArrayElement);
      return;
    case TokenType::True:
    case TokenType::False:
      m_backend.value(tok.type == TokenType::True);
      update(ParserState::ReadArrayElement);
      return;
    case TokenType::Integer:
      m_backend.value(m_backend.parse_integer(tok.text));
      update(ParserState::ReadArrayElement);
      return;
    case TokenType::Number:
      m_backend.value(m_backend.parse_number(tok.text));
      update(ParserState::ReadArrayElement);
      return;
    case TokenType::StringLiteral:
      m_backend.value(m_backend.remove_quotes(tok.text));
      update(ParserState::ReadArrayElement);
      return;
    case TokenType::RBracket:
      m_backend.end_array();
      leave();
      return;
    default:
      throw std::runtime_error{ "Invalid input in 'ParsingArray' state" };
      break;
    }
  }

  void StateReadArrayElement(const Token& tok)
  {
    switch (tok.type)
    {
    case TokenType::RBracket:
      m_backend.end_array();
      leave();
      return;
    case TokenType::Comma:
      update(ParserState::ReadArraySeparator);
      return;
    default:
      throw std::runtime_error{ "Invalid input in 'ReadArrayElement' state" };
      break;
    }
  }

  void StateReadArraySeparator(const Token& tok)
  {
    update(ParserState::ParsingArray);
    return StateParsingArray(tok);
  }

private:
  Backend m_backend;
  std::vector<ParserState> m_states;
  std::vector<Token> m_buffer;
};

} // namespace json

#include "json-default-parser-backend.h"

namespace json
{

inline json::Json parse(const std::string& str)
{
  Tokenizer<DefaultTokenizerBackend> tokenizer;
  auto& buffer = tokenizer.backend().token_buffer;
  tokenizer.write(str);
  tokenizer.done();

  ParserMachine<DefaultParserBackend> parser;
  
  for (const auto& tok : buffer)
  {
    parser.write(tok);
  }

  return parser.backend().stack.front();
}

} // namespace json

#endif // !JSONTOOLKIT_PARSING_H