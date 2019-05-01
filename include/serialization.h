
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

class Codec;

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

#endif // !LIBJSON_SERIALIZATION_H