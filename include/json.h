
#include <map>
#include <string>
#include <vector>

namespace json
{

using reference_counter_type = int;
using integer_type = int;
using number_type = double;
using string_type = std::string;

enum class ValueType
{
  Null = 0,
  Boolean,
  Integer,
  Number,
  String,
  Array,
  Object,
};


namespace detail
{

class Value
{
public:
  reference_counter_type ref;
public:
  Value();
  Value(const Value & other) = delete;
  virtual ~Value();

  virtual ValueType type() const = 0;
  virtual Value * clone() const = 0;
  virtual bool eq(const Value & other) const = 0;

  Value & operator=(const Value & other) = delete;
};

} // namespace detail

class Array;
class Object;

class Value
{
public:
  Value();
  Value(bool b);
  Value(integer_type n);
  Value(number_type x);
  Value(const string_type & str);
  Value(string_type && str);
  Value(const Value & other);
  ~Value();

  bool isNull() const;
  ValueType type() const;

  Value clone() const;
  
  bool toBool() const;
  integer_type toInteger() const;
  number_type toNumber() const;
  string_type toString() const;

  Array toArray() const;
  std::size_t length() const;
  Value getArrayValueAt(int index) const;
  void push(const Value & val);
  void insertArrayValueAt(int index, const Value & val);

  Object toObject() const;
  Value getObjectEntryValue(const std::string & name);
  void insertObjectEntryValue(const std::string & name, const Value & val);

  Value & operator=(const Value & other);
  Value & operator=(bool val);
  Value & operator=(integer_type val);
  Value & operator=(number_type val);
  Value & operator=(const char * str);
  Value & operator=(const string_type & val);
  Value & operator=(string_type && val);

  Value & operator[](int index);
  const Value & operator[](int index) const;

  Value & operator[](const std::string & name);
  const Value & operator[](const std::string & name) const;

  bool operator==(const std::nullptr_t & rhs) const;
  bool operator==(bool rhs) const;
  bool operator==(integer_type rhs) const;
  bool operator==(number_type rhs) const;
  bool operator==(const string_type & rhs) const;
  bool operator==(const Value & other) const;
  inline bool operator!=(const std::nullptr_t & rhs) const { return !(operator==(rhs)); }
  inline bool operator!=(const bool & rhs) const { return !(operator==(rhs)); }
  inline bool operator!=(const integer_type & rhs) const { return !(operator==(rhs)); }
  inline bool operator!=(const number_type & rhs) const { return !(operator==(rhs)); }
  inline bool operator!=(const string_type & rhs) const { return !(operator==(rhs)); }
  inline bool operator!=(const Value & other) const { return !(operator==(other)); }

public:
  Value(detail::Value *impl);

protected:
  detail::Value *mImpl;
};

inline bool operator==(const std::nullptr_t & lhs, const Value & rhs)
{
  return rhs.operator==(lhs);
}

inline bool operator==(const bool & lhs, const Value & rhs)
{
  return rhs.operator==(lhs);
}

inline bool operator==(const integer_type & lhs, const Value & rhs)
{
  return rhs.operator==(lhs);
}

inline bool operator==(const number_type & lhs, const Value & rhs)
{
  return rhs.operator==(lhs);
}

inline bool operator==(const string_type & lhs, const Value & rhs)
{
  return rhs.operator==(lhs);
}

inline bool operator!=(const std::nullptr_t & lhs, const Value & rhs)
{
  return rhs.operator!=(lhs);
}

inline bool operator!=(const bool & lhs, const Value & rhs)
{
  return rhs.operator!=(lhs);
}

inline bool operator!=(const integer_type & lhs, const Value & rhs)
{
  return rhs.operator!=(lhs);
}

inline bool operator!=(const number_type & lhs, const Value & rhs)
{
  return rhs.operator!=(lhs);
}

inline bool operator!=(const string_type & lhs, const Value & rhs)
{
  return rhs.operator!=(lhs);
}


class Array : public Value
{
public:
  Array(const Array & other);
  ~Array();

  typedef std::vector<Value> underlying_type;

  typename underlying_type::iterator begin();
  typename underlying_type::const_iterator begin() const;

  typename underlying_type::iterator end();
  typename underlying_type::const_iterator end() const;

  const underlying_type::value_type & front() const;
  underlying_type::value_type & front();

  const underlying_type::value_type & back() const;
  underlying_type::value_type & back();

protected:
  friend class Value;
  friend Array newArray();
  Array(detail::Value *impl);
};

Array newArray();

class Object : public Value
{
public:
  typedef std::map<std::string, Value> underlying_type;

  typename underlying_type::iterator begin();
  typename underlying_type::const_iterator begin() const;

  typename underlying_type::iterator end();
  typename underlying_type::const_iterator end() const;

protected:
  friend class Value;
  friend class Object newObject();
  Object(detail::Value *impl);
};

Object newObject();


template<typename T>
Value write(const T & val);
template<>
inline Value write<bool>(const bool & b)
{
  return Value{ b };
}

template<typename T>
T read(const Value & val);
template<>
inline bool read<bool>(const Value & val)
{
  return val.toBool();
}

namespace detail
{

class Boolean : public Value
{
public:
  bool value;
public:
  Boolean(bool val);
  ~Boolean();

  ValueType type() const override;
  Value * clone() const override;
  bool eq(const Value & other) const override;
};

class Integer : public Value
{
public:
  integer_type value;
public:
  Integer(integer_type val);
  ~Integer();

  ValueType type() const override;
  Value * clone() const override;
  bool eq(const Value & other) const override;
};

class Number : public Value
{
public:
  number_type value;
public:
  Number(number_type val);
  ~Number();

  ValueType type() const override;
  Value * clone() const override;
  bool eq(const Value & other) const override;
};

class String : public Value
{
public:
  string_type value;
public:
  String(const string_type & val);
  String(string_type && val);
  ~String();

  ValueType type() const override;
  Value * clone() const override;
  bool eq(const Value & other) const override;
};

class Array : public Value
{
public:
  json::Array::underlying_type value;
public:
  Array(const json::Array::underlying_type & val);
  Array(json::Array::underlying_type && val);
  ~Array();

  ValueType type() const override;
  Value * clone() const override;
  bool eq(const Value & other) const override;
};

class Object : public Value
{
public:
  json::Object::underlying_type value;
public:
  Object(const json::Object::underlying_type & val);
  Object(json::Object::underlying_type && val);
  ~Object();

  ValueType type() const override;
  Value * clone() const override;
  bool eq(const Value & other) const override;
};

} // namespace detail

} // namespace json