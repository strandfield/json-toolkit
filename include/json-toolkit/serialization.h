// Copyright (C) 2019 Vincent Chambrin
// This file is part of the json-toolkit library
// For conditions of distribution and use, see copyright notice in LICENSE

#ifndef JSONTOOLKIT_SERIALIZATION_H
#define JSONTOOLKIT_SERIALIZATION_H

#include "json-toolkit/json.h"

#include <unordered_map>
#include <stdexcept>

#if __cplusplus >= 201703L || defined(JSONTOOLKIT_CXX17)
#include <optional>
#include <variant>
#endif // __cplusplus >= 201703L || defined(JSONTOOLKIT_CXX17)

namespace json
{

typedef size_t hash_code_t;

class Serializer;

namespace serialization
{

template<typename T>
struct decoder
{
  static void decode(Serializer&, const Json&, T&)
  {
    throw std::runtime_error{ "No decoder" };
  }
};

template<typename T>
struct encoder
{
  static Json encode(Serializer&, const T&)
  {
    throw std::runtime_error{ "No encoder" };
  }
};

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

#if __cplusplus >= 201703L || defined(JSONTOOLKIT_CXX17)

namespace serialization
{

template<typename VariantType, size_t Index>
struct variant_decoder;

template<typename...Args, size_t Index>
struct variant_decoder<std::variant<Args...>, Index>
{
  static void decode(Serializer& s, size_t index, const Json& data, std::variant<Args...>& value)
  {
    if constexpr (Index == sizeof...(Args))
    {
      throw std::runtime_error{ "Could not decode variant" };
    }
    else
    {
      using Type = std::variant_alternative_t<Index, std::variant<Args...>>;

      if (index == Index)
        value = s.decode<Type>(data);
      else
        variant_decoder<std::variant<Args...>, Index + 1>::decode(s, index, data, value);
    }
  }
};

template<typename...Args>
struct decoder<std::variant<Args...>>
{
  static void decode(Serializer& s, const Json& data, std::variant<Args...>& value)
  {
    variant_decoder<std::variant<Args...>, 0>::decode(s, data["index"].toInt(), data["value"], value);
  }
};

template<typename...Args>
struct encoder<std::variant<Args...>>
{
  static Json encode(Serializer& s, const std::variant<Args...>& values)
  {
    Json result = {};
    result["index"] = (config::integer_type) values.index();
    result["value"] = std::visit([&s](const auto & val) -> Json {
      return s.encode(val);
    }, values);
    return result;
  }
};

} // namespace serialization

#endif // __cplusplus >= 201703L || defined(JSONTOOLKIT_CXX17)

#if __cplusplus >= 201703L || defined(JSONTOOLKIT_CXX17)

namespace serialization
{

template<typename T>
struct decoder<std::optional<T>>
{
  static void decode(Serializer& s, const Json& data, std::optional<T>& value)
  {
    if (data.isNull())
      return;

    value = s.decode<T>(data);
  }
};

template<typename T>
struct encoder<std::optional<T>>
{
  static Json encode(Serializer& s, const std::optional<T>& value)
  {
    if (!value.has_value())
      return Json(nullptr);

    return s.encode(value.value());
  }
};

} // namespace serialization

#endif // __cplusplus >= 201703L || defined(JSONTOOLKIT_CXX17)

class Codec;

class Serializer
{
public:
  Serializer() = default;
  Serializer(const Serializer&) = delete;
  ~Serializer() = default;

  template<typename T>
  T decode(const Json& obj);

  template<typename T>
  Json encode(const T& value);

  void addCodec(std::unique_ptr<Codec>&& codec);
  inline void addCodec(Codec* c) { addCodec(std::unique_ptr<Codec>(c)); }

  inline const std::unordered_map<hash_code_t, std::unique_ptr<Codec>>& codecs() const { return m_codecs; }

private:
  std::unordered_map<hash_code_t, std::unique_ptr<Codec>> m_codecs;
};

class Codec
{
public:
  Codec() = default;
  virtual ~Codec() = default;

  virtual hash_code_t hash_code() const = 0;

  virtual void decode(Serializer& serializer, const Json& data, void* value) = 0;
  virtual Json encode(Serializer& serializer, void* value) = 0;
};

} // namespace json

namespace json
{

template<typename T>
inline T Serializer::decode(const Json& obj)
{
  T result;
  
  auto it = codecs().find(typeid(T).hash_code());

  if (it != codecs().end())
  {
    it->second->decode(*this, obj, (void*)& result);
    return result;
  }

  serialization::decoder<T>::decode(*this, obj, result);

  return result;
}

template<typename T>
inline Json Serializer::encode(const T& value)
{
  auto it = codecs().find(typeid(T).hash_code());

  if (it != codecs().end())
    return it->second->encode(*this, (void*)& value);

  return serialization::encoder<T>::encode(*this, value);
}

inline void Serializer::addCodec(std::unique_ptr<Codec>&& codec)
{
  m_codecs[codec->hash_code()] = std::move(codec);
}

} // namespace json

namespace json
{

namespace details
{

class ObjectField
{
public:
  ObjectField(const std::string& mn) : member_name_(mn), optional_(false) { }
  virtual ~ObjectField() = default;

  virtual void decode_field(Serializer& serializer, const Json& object_data, const Json& field_data, void* value) = 0;
  virtual Json encode_field(Serializer& serializer, void* value) = 0;

  std::string member_name_;
  bool optional_;
};

template<typename T, typename M>
class SimpleMemberField : public ObjectField
{
public:
  M T::*member;

public:
  SimpleMemberField(const std::string& mn, M T::*mem_ptr) : ObjectField(mn), member(mem_ptr) { }
  ~SimpleMemberField() = default;

  void decode_field(Serializer& serializer, const Json& object_data, const Json& field_data, void* value) override
  {
    T& object = *static_cast<T*>(value);
    object.*member = serializer.decode<M>(field_data);
  }

  Json encode_field(Serializer& serializer, void* value) override
  {
    const T& object = *static_cast<const T*>(value);
    return serializer.encode(object.*member);
  }
};

template<typename T, typename M, typename Getter, typename Setter>
class MemberField : public ObjectField
{
public:
  Getter getter;
  Setter setter;

public:
  MemberField(const std::string& mn, Getter get, Setter set) : ObjectField(mn), getter(get), setter(set) { }
  ~MemberField() = default;

  void decode_field(Serializer& serializer, const Json& object_data, const Json& field_data, void* value) override
  {
    T& object = *static_cast<T*>(value);
    (object.*setter)(serializer.decode<M>(field_data));
  }

  Json encode_field(Serializer& serializer, void* value) override
  {
    const T& object = *static_cast<const T*>(value);
    return serializer.encode((object.*getter)());
  }
};

} // namespace details

class ObjectCodec : public Codec
{
public:
  template<typename T>
  static ObjectCodec* create();

  inline const std::map<std::string, std::unique_ptr<details::ObjectField>>& fields() const { return m_fields; }

  template<typename T, typename M>
  void addField(const std::string& name, M T::*member);

  template<typename T, typename M>
  void addField(const std::string& name, M(T::*getter)() const, void (T::*setter)(M));

protected:
  std::map<std::string, std::unique_ptr<details::ObjectField>> m_fields;
};

template<typename T>
class GenericObjectCodec : public ObjectCodec
{
public:

  hash_code_t hash_code() const override { return typeid(T).hash_code(); }

  void decode(Serializer& serializer, const Json& data, void* value) override
  {
    T& object = *static_cast<T*>(value);

    for (auto it = fields().begin(); it != fields().end(); ++it)
    {
      details::ObjectField* field = it->second.get();

      Json field_data = data[field->member_name_];

      if (field_data.isNull())
      {
        if (!field->optional_)
          throw std::runtime_error{ "Missing required field" };
      }
      else
      {
        field->decode_field(serializer, data, field_data, value);
      }
    }
  }

  Json encode(Serializer& serializer, void* value) override
  {
    const T& object = *static_cast<T*>(value);

    Json result = {};

    for (auto it = fields().begin(); it != fields().end(); ++it)
    {
      details::ObjectField* field = it->second.get();
      result[field->member_name_] = field->encode_field(serializer, value);
    }

    return result;
  }
};

} // namespace json

namespace json
{

template<typename T>
inline ObjectCodec* ObjectCodec::create()
{
  return new GenericObjectCodec<T>();
}

template<typename T, typename M>
inline void ObjectCodec::addField(const std::string& name, M T::*member)
{
  assert(hash_code() == typeid(T).hash_code());
  m_fields[name] = std::unique_ptr<details::ObjectField>(new details::SimpleMemberField<T, M>(name, member));
}

template<typename T, typename M>
inline void ObjectCodec::addField(const std::string& name, M(T::*getter)() const, void (T::*setter)(M))
{
  assert(hash_code() == typeid(T).hash_code());
  using BaseMemberType = typename std::remove_const<typename std::remove_reference<M>::type>::type;
  m_fields[name] = std::unique_ptr<details::ObjectField>(new details::MemberField<T, BaseMemberType, decltype(getter), decltype(setter)>(name, getter, setter));
}

} // namespace json

#endif // !JSONTOOLKIT_SERIALIZATION_H