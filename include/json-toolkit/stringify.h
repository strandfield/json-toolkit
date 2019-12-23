// Copyright (C) 2019 Vincent Chambrin
// This file is part of the json-toolkit library
// For conditions of distribution and use, see copyright notice in LICENSE

#ifndef JSONTOOLKIT_STRINGIFY_H
#define JSONTOOLKIT_STRINGIFY_H

#include "json-toolkit/json.h"

namespace json
{

enum StringifyOptions {
  None = 0,
};

std::string stringify(const json::Json& data, StringifyOptions options = None);

enum class WriterState
{
  Idle,
  StartedObject,
  WroteObjectKey,
  WroteObjectValue,
  StartedArray,
  WroteArrayValue,
};

template<typename Backend>
class GenericWriter
{
public:
  GenericWriter()
    : m_key_quotes(CharCategory::Invalid), 
    m_depth(0)
  {
    m_states.push_back(WriterState::Idle);
  }

  inline WriterState state() const { return m_states.back(); }
  inline const std::vector<WriterState>& stack() const { return m_states; }

  inline Backend& backend() { return m_backend; }

  void value(std::nullptr_t)
  {
    writeArraySeparator();
    backend() << nullptr;
    update();
  }

  void value(bool val)
  {
    writeArraySeparator();
    backend() << val;
    update();
  }

  void value(int val)
  {
    writeArraySeparator();
    backend() << val;
    update();
  }

  void value(double val)
  {
    writeArraySeparator();
    backend() << val;
    update();
  }

  void value(const std::string& str)
  {
    writeArraySeparator();
    backend() << CharCategory::DoubleQuote << str << CharCategory::DoubleQuote;
    update();
  }

  void start_object()
  {
    writeArraySeparator();

    backend() << CharCategory::LBrace;

    enter(WriterState::StartedObject);
  }

  void key(const std::string& str)
  {
    if (state() == WriterState::WroteObjectValue)
    {
      backend() << CharCategory::Comma << CharCategory::NewLine;
    }
    else if (state() == WriterState::StartedObject)
    {
      backend() << CharCategory::NewLine;
    }
    else
    {
      throw std::runtime_error{ "Invalid writer state" };
    }

    indent();

    backend() << CharCategory::DoubleQuote <<  str << CharCategory::DoubleQuote  << CharCategory::Colon << CharCategory::Space;

    update(WriterState::WroteObjectKey);
  }

  void end_object()
  {
    if (state() == WriterState::StartedObject)
    {
      backend() << CharCategory::RBrace;
    }
    else if (state() == WriterState::WroteObjectValue)
    {
      backend() << CharCategory::NewLine;
      indent(-1);
      backend() << CharCategory::RBrace;
    }

    leave();
  }

  void start_array()
  {
    writeArraySeparator();

    backend() << CharCategory::LBracket;

    enter(WriterState::StartedArray);
  }

  void end_array()
  {
    if (state() == WriterState::StartedArray || state() == WriterState::WroteArrayValue)
    {
      backend() << CharCategory::RBracket;
    }
    else
    {
      throw std::runtime_error("Invalid state in end_array");
    }

    leave();
  }

protected:

  void update(WriterState ws)
  {
    m_states.back() = ws;
  }

  void enter(WriterState ws)
  {
    m_states.push_back(ws);
  }

  void leave()
  {
    m_states.pop_back();

    if (state() == WriterState::WroteObjectKey)
      update(WriterState::WroteObjectValue);
    else if (state() == WriterState::StartedArray)
      update(WriterState::WroteArrayValue);
  }

  void indent(int delta = 0)
  {
    for (size_t i(0); i < stack().size() - 1 + delta; ++i)
    {
      backend() << CharCategory::Space << CharCategory::Space;
    }
  }

  // Update state after writing a value
  void update()
  {
    if (state() == WriterState::StartedArray)
      update(WriterState::WroteArrayValue);
    else if (state() == WriterState::WroteObjectKey)
      update(WriterState::WroteObjectValue);
  }

  void writeArraySeparator()
  {
    if (state() == WriterState::WroteArrayValue)
    {
      backend() << CharCategory::Comma << CharCategory::Space;
    }
  }

private:
  CharCategory m_key_quotes;
  int m_depth;
  Backend m_backend;
  std::vector<WriterState> m_states;
};

} // namespace json

#include "json-default-writer-backend.h"

namespace json
{

namespace details
{

inline void write(GenericWriter<DefaultWriterBackend>& writer, const json::Json& data)
{
  if (data.isArray())
  {
    writer.start_array();

    for (int i(0); i < data.length(); ++i)
    {
      write(writer, data.at(i));
    }

    writer.end_array();
  }
  else if (data.isObject())
  {
    writer.start_object();

    Object obj = data.toObject();

    for (const auto& e : obj.data())
    {
      writer.key(e.first);
      write(writer, e.second);
    }

    writer.end_object();
  }
  else if (data.isNull())
  {
    writer.value(nullptr);
  }
  else if (data.isBoolean())
  {
    writer.value(data.toBool());
  }
  else if (data.isInteger())
  {
    writer.value(data.toInt());
  }
  else if (data.isNumber())
  {
    writer.value(data.toNumber());
  }
  else if (data.isString())
  {
    writer.value(data.toString());
  }
}

} // namespace details

inline std::string stringify(const json::Json& data, StringifyOptions options)
{
  GenericWriter<DefaultWriterBackend> writer;
  details::write(writer, data);
  return writer.backend().result();
}

} // namespace json

#endif // !JSONTOOLKIT_STRINGIFY_H
