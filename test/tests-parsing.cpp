// Copyright (C) 2019 Vincent Chambrin
// This file is part of the json-toolkit library
// For conditions of distribution and use, see copyright notice in LICENSE

#include <gtest/gtest.h>

#include "json-toolkit/parsing.h"

TEST(parsing, tokenizer)
{
  using namespace json;
  
  Tokenizer<DefaultTokenizerBackend> tokenizer;
  auto& buffer = tokenizer.backend().token_buffer;

  tokenizer.write("123 hello 'str' \"haha\" ");

  ASSERT_EQ(buffer.size(), 4);
  ASSERT_EQ(buffer.front(), Token(TokenType::Integer, "123"));
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
  ASSERT_EQ(buffer.at(0), Token(TokenType::Integer, "125"));
  ASSERT_EQ(buffer.at(1), Token(TokenType::Number, "1.31"));
  ASSERT_EQ(buffer.at(2), Token(TokenType::Number, "1e+28"));
  ASSERT_EQ(buffer.at(3), Token(TokenType::Number, "-2.45e-27"));
}

TEST(parsing, tokenizer_exceptions)
{
  using namespace json;

  Tokenizer<DefaultTokenizerBackend> tokenizer;
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

TEST(parsing, parser_machine_tokens)
{
  using namespace json;

  ParserMachine<DefaultParserBackend> parser;

  std::vector<Token> tokens{
    Token(TokenType::LBrace),
    Token(TokenType::Identifier, "name"),
    Token(TokenType::Colon),
    Token(TokenType::StringLiteral, "'Alice'"),
    Token(TokenType::Comma),
    Token(TokenType::Identifier, "age"),
    Token(TokenType::Colon),
    Token(TokenType::Integer, "18"),
    Token(TokenType::RBrace),
  };

  for (const auto& tok : tokens)
    parser.write(tok);

  ASSERT_TRUE(parser.state() == ParserState::Idle);
  ASSERT_EQ(parser.backend().stack.size(), 1);
  ASSERT_TRUE(parser.backend().stack.front().isObject());

  json::Object obj = parser.backend().stack.front().toObject();

  ASSERT_EQ(obj.data().size(), 2);
  ASSERT_EQ(obj["name"], "Alice");
  ASSERT_EQ(obj["age"], 18);
}

TEST(parsing, parser_machine_string_1)
{
  using namespace json;

  std::string input =
    "  {                                       "
    "    name: 'Alice',                        "
    "    age: 18,                              "
    "    code: true,                           "
    "    languages: ['C++', 'JSON'],           "
    "    pi: 3.14159,                          "
    "    book: {                               "
    "      name: 'The Story of Alice& Bob',    "
    "      year: 2019,                         "
    "      isbn: '978 - 0321958310'            "
    "    }                                     "
    "  }                                       ";

  Tokenizer<DefaultTokenizerBackend> tokenizer;
  auto& buffer = tokenizer.backend().token_buffer;
  tokenizer.write(input);

  ParserMachine<DefaultParserBackend> parser;

  for (const auto& tok : buffer)
    parser.write(tok);

  ASSERT_TRUE(parser.state() == ParserState::Idle);
  ASSERT_EQ(parser.backend().stack.size(), 1);
  ASSERT_TRUE(parser.backend().stack.front().isObject());

  json::Object obj = parser.backend().stack.front().toObject();

  ASSERT_EQ(obj.data().size(), 6);
  ASSERT_EQ(obj["name"], "Alice");
  ASSERT_EQ(obj["age"], 18);
  ASSERT_EQ(obj["code"], true);
  ASSERT_TRUE(obj["languages"].isArray());
  ASSERT_TRUE(obj["book"].isObject());
  ASSERT_EQ(obj["book"]["isbn"], "978 - 0321958310");
}

TEST(parsing, parser_machine_string_2)
{
  using namespace json;

  std::string input = " [1, 2, [true, false], {}, 3.14] ";

  Tokenizer<DefaultTokenizerBackend> tokenizer;
  auto& buffer = tokenizer.backend().token_buffer;
  tokenizer.write(input);

  ParserMachine<DefaultParserBackend> parser;

  for (const auto& tok : buffer)
    parser.write(tok);

  ASSERT_TRUE(parser.state() == ParserState::Idle);
  ASSERT_EQ(parser.backend().stack.size(), 1);
  ASSERT_TRUE(parser.backend().stack.front().isArray());

  json::Array vec = parser.backend().stack.front().toArray();

  ASSERT_EQ(vec.length(), 5);
  ASSERT_EQ(vec.at(0), 1);
  ASSERT_EQ(vec.at(1), 2);
  ASSERT_TRUE(vec.at(2).isArray());
  ASSERT_EQ(vec.at(2).length(), 2);
  ASSERT_EQ(vec.at(3), json::Object());
  ASSERT_EQ(vec.at(4), 3.14);
}

TEST(parsing, parser_machine_exceptions)
{
  using namespace json;

  ParserMachine<DefaultParserBackend> parser;

  // [}]
  parser.write(Token(TokenType::LBracket));
  ASSERT_ANY_THROW(parser.write(Token(TokenType::RBrace)));
  parser.write(Token(TokenType::RBracket));
  ASSERT_EQ(parser.state(), ParserState::Idle);
  parser.backend().stack.clear();

  // { name: : 'Bob',}
  parser.write(Token(TokenType::LBrace));
  parser.write(Token(TokenType::Identifier, "name"));
  parser.write(Token(TokenType::Colon));
  ASSERT_ANY_THROW(parser.write(Token(TokenType::Colon)));
  parser.write(Token(TokenType::StringLiteral, "'Bob'"));
  parser.write(Token(TokenType::Comma));
  parser.write(Token(TokenType::RBrace));
  ASSERT_EQ(parser.state(), ParserState::Idle);
  parser.backend().stack.clear();
}
