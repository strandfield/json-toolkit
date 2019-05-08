
#ifndef LIBJSON_JSON_H
#define LIBJSON_JSON_H

#include "json-global-defs.h"

#if defined(LIBJSON_HAS_CONFIG)
#  include "json-config.h"
#else
#  include "json-default-config.h"
#endif // defined(LIBJSON_HAS_CONFIG)

#include <memory>

namespace json
{

enum class JsonType
{
  Null = 0,
  Boolean,
  Integer,
  Number,
  String,
  Array,
  Object,
};

namespace details
{

class Node
{
public:
  Node() = default;
  Node(const Node&) = delete;
  virtual ~Node() = default;

  virtual JsonType type() const = 0;

  Node& operator=(const Node&) = delete;
};

} // namespace details

class Array;
class Object;

class Json
{
public:
  Json();
  Json(const Json&) = default;
  ~Json() = default;

  Json(nullptr_t);
  Json(bool bval);
  Json(config::integer_type ival);
  Json(config::number_type nval);
  Json(const config::string_type& str);
  Json(const char* str);

  inline Json(const std::shared_ptr<details::Node>& impl) : d(impl) { }

  inline JsonType type() const { return d->type(); }

  inline bool isNull() const { return type() == JsonType::Null; }
  inline bool isBoolean() const { return type() == JsonType::Boolean; }
  inline bool isInteger() const { return type() == JsonType::Integer; }
  inline bool isNumber() const { return type() == JsonType::Number; }
  inline bool isString() const { return type() == JsonType::String; }
  inline bool isArray() const { return type() == JsonType::Array; }
  inline bool isObject() const { return type() == JsonType::Object; }

  /* Value interface */
  bool toBool() const;
  config::integer_type toInt() const;
  config::number_type toNumber() const;
  const config::string_type& toString() const;

  /* Array interface */
  int length() const;
  Json at(int index) const;
  Json& operator[](int index);
  void push(const Json& val);
  Array toArray() const;

  /* Object interface */
  Json& operator[](const config::string_type& key);
  Json operator[](const config::string_type& key) const;
  Object toObject() const;

  inline const std::shared_ptr<details::Node>& impl() const { return d; }

  Json& operator=(const Json&) = default;

  Json& operator=(nullptr_t);
  Json& operator=(bool val);
  Json& operator=(config::integer_type val);
  Json& operator=(config::number_type val);
  Json& operator=(const config::string_type& str);
  Json& operator=(const char* str);

  inline bool operator==(nullptr_t) const { return type() == JsonType::Null; }

protected:
  std::shared_ptr<details::Node> d;
};

static const Json null = Json(nullptr);

int compare(const Json& lhs, const Json& rhs);

bool operator==(const Json& lhs, const Json& rhs);
inline bool operator!=(const Json& lhs, const Json& rhs) { return !(lhs == rhs); }

namespace details
{

class NullNode : public Node
{
public:
  NullNode() = default;
  ~NullNode() = default;

  JsonType type() const override { return JsonType::Null; }

  static std::shared_ptr<NullNode> get()
  {
    static std::shared_ptr<NullNode> static_instance = std::make_shared<NullNode>();
    return static_instance;
  }
};

class BooleanNode : public Node
{
public:
  bool value;

public:
  BooleanNode(bool val) : value(val) { }
  ~BooleanNode() = default;

  JsonType type() const override { return JsonType::Boolean; }

  static std::shared_ptr<BooleanNode> True()
  {
    static std::shared_ptr<BooleanNode> static_instance = std::make_shared<BooleanNode>(true);
    return static_instance;
  }

  static std::shared_ptr<BooleanNode> False()
  {
    static std::shared_ptr<BooleanNode> static_instance = std::make_shared<BooleanNode>(false);
    return static_instance;
  }
};

class IntegerNode : public Node
{
public:
  config::integer_type value;

public:
  IntegerNode(config::integer_type val) : value(val) { }
  ~IntegerNode() = default;

  JsonType type() const override { return JsonType::Integer; }
};

class NumberNode : public Node
{
public:
  config::number_type value;

public:
  NumberNode(config::number_type val) : value(val) { }
  ~NumberNode() = default;

  JsonType type() const override { return JsonType::Number; }
};

class StringNode : public Node
{
public:
  config::string_type value;

public:
  StringNode(config::string_type val) : value(val) { }
  ~StringNode() = default;

  JsonType type() const override { return JsonType::String; }
};

class ArrayNode : public Node
{
public:
  config::array_type<Json> value;

public:
  ArrayNode() = default;
  ArrayNode(config::array_type<Json>&& val) : value(std::move(val)) { }
  ~ArrayNode() = default;

  JsonType type() const override { return JsonType::Array; }
};

class ObjectNode : public Node
{
public:
  config::map_type<config::string_type, Json> value;

public:
  ObjectNode() = default;
  ObjectNode(config::map_type<config::string_type, Json>&& val) : value(std::move(val)) { }
  ~ObjectNode() = default;

  JsonType type() const override { return JsonType::Object; }
};

} // namespace details

class Array : public Json
{
public:
  Array();
  Array(const Array&) = default;
  ~Array() = default;

  Array(const std::shared_ptr<details::Node>& impl);

  config::array_type<Json>& data();
  const config::array_type<Json>& data() const;

  inline config::array_type<Json>& operator*() { return data(); }
  inline const config::array_type<Json>& operator*() const { return data(); }

  inline config::array_type<Json>* operator->() { return &data(); }
  inline const config::array_type<Json>* operator->() const { return &data(); }

  Array& operator=(const Array&) = default;
};

class Object : public Json
{
public:
  Object();
  Object(const Object&) = default;
  ~Object() = default;

  Object(const std::shared_ptr<details::Node>& obj);

  config::map_type<config::string_type, Json>& data();
  const config::map_type<config::string_type, Json>& data() const;

  inline config::map_type<config::string_type, Json>& operator*() { return data(); }
  inline const config::map_type<config::string_type, Json>& operator*() const { return data(); }

  inline config::map_type<config::string_type, Json>* operator->() { return &data(); }
  inline const config::map_type<config::string_type, Json>* operator->() const { return &data(); }

  Object& operator=(const Object&) = default;
};

} // namespace json

#include <cassert>
#include <stdexcept>

namespace json
{

inline Json::Json() : d(std::make_shared<details::ObjectNode>()) { }
inline Json::Json(nullptr_t) : d(details::NullNode::get()) { }
inline Json::Json(bool bval) : d(std::make_shared<details::BooleanNode>(bval)) { }
inline Json::Json(config::integer_type ival) : d(std::make_shared<details::IntegerNode>(ival)) { }
inline Json::Json(config::number_type nval) : d(std::make_shared<details::NumberNode>(nval)) { }
inline Json::Json(const config::string_type& str) : d(std::make_shared<details::StringNode>(str)) { }
inline Json::Json(const char* str) : d(std::make_shared<details::StringNode>(str)) { }

inline bool Json::toBool() const
{
  assert(isBoolean());
  return static_cast<const details::BooleanNode*>(d.get())->value;
}

inline config::integer_type Json::toInt() const
{
  assert(isInteger());
  return static_cast<const details::IntegerNode*>(d.get())->value;
}

inline config::number_type Json::toNumber() const
{
  assert(isNumber());
  return static_cast<const details::NumberNode*>(d.get())->value;
}

inline const config::string_type& Json::toString() const
{
  assert(isString());
  return static_cast<const details::StringNode*>(d.get())->value;
}

inline int Json::length() const
{
  assert(isArray());
  return (int) static_cast<const details::ArrayNode*>(d.get())->value.size();
}

inline Json Json::at(int index) const
{
  assert(isArray());
  return static_cast<const details::ArrayNode*>(d.get())->value.at(index);
}

inline Json& Json::operator[](int index)
{
  assert(isArray());
  return static_cast<details::ArrayNode*>(d.get())->value[index];
}

inline void Json::push(const Json& val)
{
  assert(isArray());
  static_cast<details::ArrayNode*>(d.get())->value.push_back(val);
}

inline Array Json::toArray() const
{
  return Array(d);
}

inline Json& Json::operator[](const config::string_type& key)
{
  assert(isObject());
  return static_cast<details::ObjectNode*>(d.get())->value[key];
}

inline Json Json::operator[](const config::string_type& key) const
{
  assert(isObject());
  auto* impl = static_cast<details::ObjectNode*>(d.get());
  auto it = impl->value.find(key);
  if (it != impl->value.end())
    return it->second;
  return Json();
}

inline Object Json::toObject() const
{
  return Object(d);
}

inline Json& Json::operator=(nullptr_t)
{
  d = details::NullNode::get();
  return *this;
}

inline Json& Json::operator=(bool val)
{
  d = val ? details::BooleanNode::True() : details::BooleanNode::False();
  return *this;
}

inline Json& Json::operator=(config::integer_type val)
{
  d = std::make_shared<details::IntegerNode>(val);
  return *this;
}

inline Json& Json::operator=(config::number_type val)
{
  d = std::make_shared<details::NumberNode>(val);
  return *this;
}

inline Json& Json::operator=(const config::string_type& str)
{
  d = std::make_shared<details::StringNode>(str);
  return *this;
}

inline Json& Json::operator=(const char* str)
{
  d = std::make_shared<details::StringNode>(str);
  return *this;
}

template<typename T>
int number_compare(const T* lhs_node, const T* rhs_node)
{
  const auto diff = lhs_node->value - rhs_node->value;
  return (0 < diff) - (diff < 0);
}

inline int array_compare(const Array& lhs, const Array& rhs)
{
  const int size_diff = lhs.length() - rhs.length();

  if (size_diff != 0)
    return (0 < size_diff) - (size_diff < 0);

  for (int i(0); i < lhs.length(); ++i)
  {
    const int c = json::compare(lhs.at(i), rhs.at(i));

    if (c != 0)
      return c;
  }

  return 0;
}

inline int object_compare(const Object& lhs, const Object& rhs)
{
  const int size_diff = static_cast<int>(lhs.data().size()) - static_cast<int>(rhs.data().size());

  if (size_diff != 0)
    return (0 < size_diff) - (size_diff < 0);

  auto lhs_it = lhs.data().begin();
  auto rhs_it = rhs.data().begin();

  for (; lhs_it != lhs.data().end(); ++lhs_it, ++rhs_it)
  {
    int c = config::string_compare(lhs_it->first, rhs_it->first);

    if (c != 0)
      return c;

    c = json::compare(lhs_it->second, rhs_it->second);

    if (c != 0)
      return c;
  }

  return 0;
}

inline int compare(const Json& lhs, const Json& rhs)
{
  const int type_diff = static_cast<int>(lhs.type()) - static_cast<int>(rhs.type());

  if (type_diff != 0)
    return (0 < type_diff) - (type_diff < 0);

  switch (lhs.type())
  {
  case JsonType::Null:
    return 0;
  case JsonType::Boolean:
    return static_cast<int>(static_cast<const details::BooleanNode*>(lhs.impl().get())->value)
      - static_cast<int>(static_cast<const details::BooleanNode*>(rhs.impl().get())->value);
  case JsonType::Integer:
    return number_compare(static_cast<const details::IntegerNode*>(lhs.impl().get()), static_cast<const details::IntegerNode*>(rhs.impl().get()));
  case JsonType::Number:
    return number_compare(static_cast<const details::NumberNode*>(lhs.impl().get()), static_cast<const details::NumberNode*>(rhs.impl().get()));
  case JsonType::String:
    return config::string_compare(lhs.toString(), rhs.toString());
  case JsonType::Array:
    return array_compare(lhs.toArray(), rhs.toArray());
  case JsonType::Object:
    return object_compare(lhs.toObject(), rhs.toObject());
  }

  assert(false);
  throw std::runtime_error{ "json::compare() : corrupted inputs" };
}

inline bool operator==(const Json& lhs, const Json& rhs)
{
  if (lhs.impl() == rhs.impl())
    return true;

  if (lhs.type() != rhs.type())
    return false;

  return json::compare(lhs, rhs) == 0;
}

inline Array::Array() 
  : Json(std::make_shared<details::ArrayNode>())
{

}

inline Array::Array(const std::shared_ptr<details::Node>& impl)
  : Json(impl->type() == JsonType::Array ? impl : details::NullNode::get())
{

}

inline config::array_type<Json>& Array::data()
{
  assert(isArray());
  return static_cast<details::ArrayNode*>(d.get())->value;
}

inline const config::array_type<Json>& Array::data() const
{
  assert(isArray());
  return static_cast<const details::ArrayNode*>(d.get())->value;
}

inline Object::Object()
  : Json(std::make_shared<details::ObjectNode>())
{

}

inline Object::Object(const std::shared_ptr<details::Node>& obj)
  : Json(obj->type() == JsonType::Object ? obj : details::NullNode::get())
{

}

inline config::map_type<config::string_type, Json>& Object::data()
{
  assert(isObject());
  return static_cast<details::ObjectNode*>(d.get())->value;
}

inline const config::map_type<config::string_type, Json>& Object::data() const
{
  assert(isObject());
  return static_cast<const details::ObjectNode*>(d.get())->value;
}

} // namespace json

#endif // !LIBJSON_JSON_H