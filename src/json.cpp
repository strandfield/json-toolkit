#include "json.h"

#include <stdexcept>


namespace json
{

namespace detail
{

Value::Value()
  : ref(0)
{

}

Value::~Value()
{

}


Boolean::Boolean(bool val)
  : value(val)
{

}

Boolean::~Boolean()
{

}

ValueType Boolean::type() const
{
  return ValueType::Boolean;
}

Value * Boolean::clone() const
{
  return new Boolean(this->value);
}

bool Boolean::eq(const Value & other) const
{
  if (other.type() != ValueType::Boolean)
    return false;
  return dynamic_cast<const Boolean &>(other).value == this->value;
}



Integer::Integer(integer_type val)
  : value(val)
{

}

Integer::~Integer()
{

}

ValueType Integer::type() const
{
  return ValueType::Integer;
}

Value * Integer::clone() const
{
  return new Integer(this->value);
}

bool Integer::eq(const Value & other) const
{
  if (other.type() != ValueType::Integer)
    return false;
  return dynamic_cast<const Integer &>(other).value == this->value;
}



Number::Number(number_type val)
  : value(val)
{

}

Number::~Number()
{

}

ValueType Number::type() const
{
  return ValueType::Number;
}

Value * Number::clone() const
{
  return new Number(this->value);
}

bool Number::eq(const Value & other) const
{
  if (other.type() != ValueType::Number)
    return false;
  return dynamic_cast<const Number &>(other).value == this->value;
}



String::String(const string_type & val)
  : value(val)
{

}

String::String(string_type && val)
  : value(std::move(val))
{

}

String::~String()
{

}

ValueType String::type() const
{
  return ValueType::String;
}

Value * String::clone() const
{
  return new String(this->value);
}

bool String::eq(const Value & other) const
{
  if (other.type() != ValueType::String)
    return false;
  return dynamic_cast<const String &>(other).value == this->value;
}



Array::Array(const json::Array::underlying_type & val)
  : value(val)
{

}

Array::Array(json::Array::underlying_type && val)
  : value(std::move(val))
{

}

Array::~Array()
{

}

ValueType Array::type() const
{
  return ValueType::Array;
}

Value * Array::clone() const
{
  return new Array(this->value);
}

bool Array::eq(const Value & other) const
{
  if (other.type() != ValueType::Array)
    return false;
  return dynamic_cast<const Array &>(other).value == this->value;
}




Object::Object(const json::Object::underlying_type & val)
  : value(val)
{

}

Object::Object(json::Object::underlying_type && val)
  : value(std::move(val))
{

}

Object::~Object()
{

}

ValueType Object::type() const
{
  return ValueType::Object;
}

Value * Object::clone() const
{
  return new Object(this->value);
}

bool Object::eq(const Value & other) const
{
  if (other.type() != ValueType::Object)
    return false;
  return dynamic_cast<const Object &>(other).value == this->value;
}



} // namespace detail


Value::Value()
  : mImpl(nullptr)
{

}

Value::Value(bool b) 
  : mImpl(new detail::Boolean{b})
{
  mImpl->ref = 1;
}

Value::Value(integer_type n)
  : mImpl(new detail::Integer{n})
{
  mImpl->ref = 1;
}

Value::Value(number_type x)
  : mImpl(new detail::Number{ x })
{
  mImpl->ref = 1;
}

Value::Value(const string_type & str)
  : mImpl(new detail::String{str})
{
  mImpl->ref = 1;
}

Value::Value(string_type && str)
  : mImpl(new detail::String{std::move(str)})
{
  mImpl->ref = 1;
}

Value::Value(const Value & other)
  : mImpl(other.mImpl)
{
  if (mImpl != nullptr)
    ++(mImpl->ref);
}

Value::Value(detail::Value *impl)
  : mImpl(impl)
{
  if (mImpl != nullptr)
    ++(mImpl->ref);
}

Value::~Value()
{
  if (mImpl != nullptr)
  {
    if (--(mImpl->ref) == 0)
      delete mImpl;
  }
}

bool Value::isNull() const
{
  return mImpl == nullptr;
}

ValueType Value::type() const
{
  return mImpl == nullptr ? ValueType::Null : mImpl->type();
}

Value Value::clone() const
{
  if (mImpl != nullptr)
    return Value{ mImpl->clone() };
  return Value{};
}

bool Value::toBool() const
{
  if (mImpl == nullptr)
    throw std::runtime_error{ "Null json value cannot be converted to bool" };
  const detail::Boolean *self = dynamic_cast<const detail::Boolean*>(mImpl);
  if(self == nullptr)
    throw std::runtime_error{ "Non-bool json value cannot be converted to bool" };
  return self->value;
}

integer_type Value::toInteger() const
{
  if (mImpl == nullptr)
    throw std::runtime_error{ "Null json value cannot be converted to integer_type" };
  const detail::Integer *self = dynamic_cast<const detail::Integer*>(mImpl);
  if (self == nullptr)
    throw std::runtime_error{ "Non-integer json value cannot be converted to integer_type" };
  return self->value;
}

number_type Value::toNumber() const
{
  if (mImpl == nullptr)
    throw std::runtime_error{ "Null json value cannot be converted to number_type" };
  const detail::Number *self = dynamic_cast<const detail::Number*>(mImpl);
  if (self == nullptr)
    throw std::runtime_error{ "Non-number json value cannot be converted to number_type" };
  return self->value;
}

string_type Value::toString() const
{
  if (mImpl == nullptr)
    throw std::runtime_error{ "Null json value cannot be converted to string_type" };
  const detail::String *self = dynamic_cast<const detail::String*>(mImpl);
  if (self == nullptr)
    throw std::runtime_error{ "Non-string json value cannot be converted to string_type" };
  return self->value;
}

Array Value::toArray() const
{
  if (mImpl == nullptr)
    throw std::runtime_error{ "Null json value cannot be converted to Array" };
  detail::Array *self = dynamic_cast<detail::Array*>(mImpl);
  if (self == nullptr)
    throw std::runtime_error{ "Non-array json value cannot be converted to Array" };
  return Array{ self };
}

std::size_t Value::length() const
{
  if (mImpl == nullptr)
    throw std::runtime_error{ "Null json value cannot be converted to Array" };
  const detail::Array *self = dynamic_cast<const detail::Array*>(mImpl);
  if (self == nullptr)
    throw std::runtime_error{ "Non-array json value cannot be converted to Array" };
  return self->value.size();
}

Value Value::getArrayValueAt(int index) const
{
  if (mImpl == nullptr)
    throw std::runtime_error{ "Null json value cannot be converted to Array" };
  const detail::Array *self = dynamic_cast<const detail::Array*>(mImpl);
  if (self == nullptr)
    throw std::runtime_error{ "Non-array json value cannot be converted to Array" };
  if (index < 0)
    index = self->value.size() + index;
  return self->value.at(index);
}

void Value::push(const Value & val)
{
  if (mImpl == nullptr)
    throw std::runtime_error{ "Null json value cannot be converted to Array" };
  detail::Array *self = dynamic_cast<detail::Array*>(mImpl);
  if (self == nullptr)
    throw std::runtime_error{ "Non-array json value cannot be converted to Array" };
  self->value.push_back(val);
}

void Value::insertArrayValueAt(int index, const Value & val)
{
  if (mImpl == nullptr)
    throw std::runtime_error{ "Null json value cannot be converted to Array" };
  detail::Array *self = dynamic_cast<detail::Array*>(mImpl);
  if (self == nullptr)
    throw std::runtime_error{ "Non-array json value cannot be converted to Array" };
  if (index < 0)
    index = self->value.size() + index;
  self->value.insert(self->value.begin() + index, val);
}

Object Value::toObject() const
{
  if (mImpl == nullptr)
    throw std::runtime_error{ "Null json value cannot be converted to Object" };
  detail::Object *self = dynamic_cast<detail::Object*>(mImpl);
  if (self == nullptr)
    throw std::runtime_error{ "Non-object json value cannot be converted to Object" };
  return Object{ self };
}

Value Value::getObjectEntryValue(const std::string & name)
{
  if (mImpl == nullptr)
    throw std::runtime_error{ "Null json value cannot be converted to Object" };
  const detail::Object *self = dynamic_cast<const detail::Object*>(mImpl);
  if (self == nullptr)
    throw std::runtime_error{ "Non-object json value cannot be converted to Object" };
  auto it = self->value.find(name);
  if (it == self->value.end())
    throw std::runtime_error{ "Object as no entry with given name" };
  return it->second;
}

void Value::insertObjectEntryValue(const std::string & name, const Value & val)
{
  if (mImpl == nullptr)
    throw std::runtime_error{ "Null json value cannot be converted to Object" };
  detail::Object *self = dynamic_cast<detail::Object*>(mImpl);
  if (self == nullptr)
    throw std::runtime_error{ "Non-object json value cannot be converted to Object" };
  self->value[name] = val;
}

Value & Value::operator=(const Value & other)
{
  other.mImpl->ref += 1;
  if (--(mImpl->ref) == 0)
    delete mImpl;
  mImpl = other.mImpl;
  return *this;
}

Value & Value::operator=(bool val)
{
  if (mImpl == nullptr)
  {
    mImpl = new detail::Boolean{ val };
    mImpl->ref = 1;
  }
  else
  {
    detail::Boolean *self = dynamic_cast<detail::Boolean*>(mImpl);
    if (self != nullptr)
    {
      self->value = val;
    }
    else
    {
      if (--(mImpl->ref))
        delete mImpl;
      mImpl = new detail::Boolean{ val };
      mImpl->ref = 1;
    }
  }

  return *this;
}

Value & Value::operator=(integer_type val)
{
  if (mImpl == nullptr)
  {
    mImpl = new detail::Integer{ val };
    mImpl->ref = 1;
  }
  else
  {
    detail::Integer *self = dynamic_cast<detail::Integer*>(mImpl);
    if (self != nullptr)
    {
      self->value = val;
    }
    else
    {
      if (--(mImpl->ref))
        delete mImpl;
      mImpl = new detail::Integer{ val };
      mImpl->ref = 1;
    }
  }

  return *this;
}


Value & Value::operator=(number_type val)
{
  if (mImpl == nullptr)
  {
    mImpl = new detail::Number{ val };
    mImpl->ref = 1;
  }
  else
  {
    detail::Number *self = dynamic_cast<detail::Number*>(mImpl);
    if (self != nullptr)
    {
      self->value = val;
    }
    else
    {
      if (--(mImpl->ref))
        delete mImpl;
      mImpl = new detail::Number{ val };
      mImpl->ref = 1;
    }
  }

  return *this;
}

Value & Value::operator=(const char * str)
{
  return (*this) = string_type{ str };
}

Value & Value::operator=(const string_type & val)
{
  if (mImpl == nullptr)
  {
    mImpl = new detail::String{ val };
    mImpl->ref = 1;
  }
  else
  {
    detail::String *self = dynamic_cast<detail::String*>(mImpl);
    if (self != nullptr)
    {
      self->value = val;
    }
    else
    {
      if (--(mImpl->ref))
        delete mImpl;
      mImpl = new detail::String{ val };
      mImpl->ref = 1;
    }
  }

  return *this;
}

Value & Value::operator=(string_type && val)
{
  if (mImpl == nullptr)
  {
    mImpl = new detail::String{ std::move(val) };
    mImpl->ref = 1;
  }
  else
  {
    detail::String *self = dynamic_cast<detail::String*>(mImpl);
    if (self != nullptr)
    {
      self->value = std::move(val);
    }
    else
    {
      if (--(mImpl->ref))
        delete mImpl;
      mImpl = new detail::String{ std::move(val) };
      mImpl->ref = 1;
    }
  }

  return *this;
}

Value & Value::operator[](int index)
{
  if (mImpl == nullptr)
    throw std::runtime_error{ "Null json value cannot be converted to Array" };
  detail::Array *self = dynamic_cast<detail::Array*>(mImpl);
  if (self == nullptr)
    throw std::runtime_error{ "Non-array json value cannot be converted to Array" };
  if (index < 0)
    index = self->value.size() + index;
  return self->value[index];
}

const Value & Value::operator[](int index) const
{
  if (mImpl == nullptr)
    throw std::runtime_error{ "Null json value cannot be converted to Array" };
  const detail::Array *self = dynamic_cast<const detail::Array*>(mImpl);
  if (self == nullptr)
    throw std::runtime_error{ "Non-array json value cannot be converted to Array" };
  if (index < 0)
    index = self->value.size() + index;
  return self->value[index];
}

Value & Value::operator[](const std::string & name)
{
  if (mImpl == nullptr)
    throw std::runtime_error{ "Null json value cannot be converted to Object" };
  detail::Object *self = dynamic_cast<detail::Object*>(mImpl);
  if (self == nullptr)
    throw std::runtime_error{ "Non-object json value cannot be converted to Object" };
  auto it = self->value.find(name);
  if (it == self->value.end())
    return self->value[name];
  return it->second;
}

const Value & Value::operator[](const std::string & name) const
{
  if (mImpl == nullptr)
    throw std::runtime_error{ "Null json value cannot be converted to Object" };
  const detail::Object *self = dynamic_cast<const detail::Object*>(mImpl);
  if (self == nullptr)
    throw std::runtime_error{ "Non-object json value cannot be converted to Object" };
  auto it = self->value.find(name);
  if (it == self->value.end())
    throw std::runtime_error{ "Object as no entry with given name" };
  return it->second;
}

bool Value::operator==(const std::nullptr_t & rhs) const
{
  return mImpl == nullptr;
}

bool Value::operator==(bool rhs) const
{
  if (mImpl == nullptr)
    return false;
  const detail::Boolean *self = dynamic_cast<const detail::Boolean*>(mImpl);
  if (self == nullptr)
    return false;
  return self->value == rhs;
}

bool Value::operator==(integer_type rhs) const
{
  if (mImpl == nullptr)
    return false;
  const detail::Integer *self = dynamic_cast<const detail::Integer*>(mImpl);
  if (self == nullptr)
    return false;
  return self->value == rhs;
}

bool Value::operator==(number_type rhs) const
{
  if (mImpl == nullptr)
    return false;
  const detail::Number *self = dynamic_cast<const detail::Number*>(mImpl);
  if (self == nullptr)
    return false;
  return self->value == rhs;
}

bool Value::operator==(const string_type & rhs) const
{
  if (mImpl == nullptr)
    return false;
  const detail::String *self = dynamic_cast<const detail::String*>(mImpl);
  if (self == nullptr)
    return false;
  return self->value == rhs;
}

bool Value::operator==(const Value & other) const
{
  if (mImpl == nullptr)
    return other.mImpl == nullptr;
  if (other.mImpl->type() != mImpl->type())
    return false;
  return mImpl->eq(*other.mImpl);
}


Array::Array(const Array & other) : Value(other)
{

}

Array::~Array()
{

}

typename Array::underlying_type::iterator Array::begin()
{
  if (mImpl == nullptr)
    throw std::runtime_error{ "Bad array access" };
  return static_cast<detail::Array*>(mImpl)->value.begin();
}

typename Array::underlying_type::const_iterator Array::begin() const
{
  if (mImpl == nullptr)
    throw std::runtime_error{ "Bad array access" };
  return static_cast<const detail::Array*>(mImpl)->value.begin();
}

typename Array::underlying_type::iterator Array::end()
{
  if (mImpl == nullptr)
    throw std::runtime_error{ "Bad array access" };
  return static_cast<detail::Array*>(mImpl)->value.end();
}

typename Array::underlying_type::const_iterator Array::end() const
{
  if (mImpl == nullptr)
    throw std::runtime_error{ "Bad array access" };
  return static_cast<const detail::Array*>(mImpl)->value.end();
}

const Array::underlying_type::value_type & Array::front() const
{
  if (mImpl == nullptr)
    throw std::runtime_error{ "Bad array access" };
  return static_cast<const detail::Array*>(mImpl)->value.front();
}

Array::underlying_type::value_type & Array::front()
{
  if (mImpl == nullptr)
    throw std::runtime_error{ "Bad array access" };
  return static_cast<detail::Array*>(mImpl)->value.front();
}

const Array::underlying_type::value_type & Array::back() const
{
  if (mImpl == nullptr)
    throw std::runtime_error{ "Bad array access" };
  return static_cast<const detail::Array*>(mImpl)->value.back();
}

Array::underlying_type::value_type & Array::back()
{
  if (mImpl == nullptr)
    throw std::runtime_error{ "Bad array access" };
  return static_cast<detail::Array*>(mImpl)->value.back();
}

Array::Array(detail::Value *impl) : Value(impl)
{
  // TODO : assert impl->type() == ValueType::Array
}


Array newArray()
{
  return Array{ new detail::Array{Array::underlying_type{}} };
}


typename Object::underlying_type::iterator Object::begin()
{
  if (mImpl == nullptr)
    throw std::runtime_error{ "Bad object access" };
  return static_cast<detail::Object*>(mImpl)->value.begin();
}

typename Object::underlying_type::const_iterator Object::begin() const
{
  if (mImpl == nullptr)
    throw std::runtime_error{ "Bad object access" };
  return static_cast<const detail::Object*>(mImpl)->value.begin();
}

typename Object::underlying_type::iterator Object::end()
{
  if (mImpl == nullptr)
    throw std::runtime_error{ "Bad object access" };
  return static_cast<detail::Object*>(mImpl)->value.end();
}

typename Object::underlying_type::const_iterator Object::end() const
{
  if (mImpl == nullptr)
    throw std::runtime_error{ "Bad object access" };
  return static_cast<const detail::Object*>(mImpl)->value.end();
}

Object::Object(detail::Value *impl) : Value(impl)
{
  // TODO : assert impl->type() == ValueType::Object
}


Object newObject()
{
  return Object{ new detail::Object{Object::underlying_type{}} };
}

} // namespace json