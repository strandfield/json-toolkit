
#ifndef LIBJSON_SERIALIZATION_H
#define LIBJSON_SERIALIZATION_H

#include "json.h"

#include <unordered_map>
#include <stdexcept>

namespace json
{

typedef size_t hash_code_t;

class Serializer;

namespace serialization
{

template<typename T>
struct decoder;

template<typename T>
struct encoder;

template<>
struct decoder<bool>
{
  static void decode(Serializer& s, const Json& data, bool& value)
  {
    value = data.toBool();
  }
};

template<>
struct decoder<config::integer_type>
{
  static void decode(Serializer& s, const Json& data, config::integer_type& value)
  {
    value = data.toInt();
  }
};

template<>
struct decoder<config::string_type>
{
  static void decode(Serializer& s, const Json& data, config::string_type& value)
  {
    value = data.toString();
  }
};

template<>
struct decoder<config::number_type>
{
  static void decode(Serializer& s, const Json& data, config::number_type& value)
  {
    value = data.toNumber();
  }
};

template<typename T>
struct decoder<config::array_type<T>>
{
  static void decode(Serializer& s, const Json& data, config::array_type<T>& value)
  {
    if (!data.isArray())
      throw std::runtime_error{ "Serializer::decode() : decode error - not an array" };

    for (int i(0); i < data.length(); ++i)
      value.push_back(s.decode<T>(data.at(i)));
  }
};

template<>
struct encoder<bool>
{
  static Json encode(Serializer& s, const bool& value)
  {
    return Json(value);
  }
};

template<>
struct encoder<config::integer_type>
{
  static Json encode(Serializer& s, const config::integer_type& value)
  {
    return Json(value);
  }
};

template<>
struct encoder<config::string_type>
{
  static Json encode(Serializer& s, const config::string_type& value)
  {
    return Json(value);
  }
};

template<>
struct encoder<config::number_type>
{
  static Json encode(Serializer& s, const config::number_type& value)
  {
    return Json(value);
  }
};

template<typename T>
struct encoder<config::array_type<T>>
{
  static Json encode(Serializer& s, const config::array_type<T>& values)
  {
    Json result = Array();

    for (const auto& val : values)
      result.push(s.encode(val));

    return result;
  }
};

} // namespace serialization

class Serializer
{
public:
  Serializer() = default;
  Serializer(const Serializer&) = default;
  ~Serializer() = default;

  template<typename T>
  T decode(const Json& obj);

  template<typename T>
  Json encode(const T& value);
};

} // namespace json

namespace json
{

template<typename T>
inline T Serializer::decode(const Json& obj)
{
  T result;
  serialization::decoder<T>::decode(*this, obj, result);
  return result;
}

template<typename T>
inline Json Serializer::encode(const T& value)
{
  return serialization::encoder<T>::encode(*this, value);
}

} // namespace json

#endif // !LIBJSON_SERIALIZATION_H