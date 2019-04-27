
#include <gtest/gtest.h>

#include "json.h"

struct Point
{
  int x;
  int y;
};

namespace json
{
template<>
Value write<Point>(const Point & pt)
{
  json::Value ret = newObject();
  ret["x"] = pt.x;
  ret["y"] = pt.y;
  return ret;
}

template<>
Point read<Point>(const Value & pt)
{
  Point ret;
  ret.x = pt["x"].toInteger();
  ret.y = pt["y"].toInteger();
  return ret;
}

}

TEST(jsontest, types)
{
  json::Value v;
  ASSERT_TRUE(v.isNull());
  ASSERT_TRUE(v == nullptr);

  v = 5;
  ASSERT_EQ(v.type(), json::ValueType::Integer);
  ASSERT_EQ(v.toInteger(), 5);
  ASSERT_TRUE(v == 5);
  ASSERT_FALSE(v == 6);
  ASSERT_FALSE(v == true);
  ASSERT_FALSE(v == nullptr);

  v = true;
  ASSERT_EQ(v.type(), json::ValueType::Boolean);

  v = 3.0;
  ASSERT_EQ(v.type(), json::ValueType::Number);

  v = "Hello World";
  ASSERT_EQ(v.type(), json::ValueType::String);

  v = json::newArray();
  ASSERT_EQ(v.type(), json::ValueType::Array);
  ASSERT_FALSE(v.toArray().isNull());

  v = json::newObject();
  ASSERT_EQ(v.type(), json::ValueType::Object);
  ASSERT_FALSE(v.toObject().isNull());
}


TEST(jsontest, arrays)
{
  json::Array val = json::newArray();
  val.push(true);
  val.push(2);
  ASSERT_EQ(val.length(), 2);

  ASSERT_EQ(val[0], true);
  val[0] = 5;
  ASSERT_EQ(val[0], 5);

  json::Array second = json::newArray();
  second.push(5);
  second.push(2);
  ASSERT_EQ(second, val);
  second[1] = 3;
  ASSERT_NE(second, val);

  ASSERT_NE(val, true);
  ASSERT_NE(val, 5);
  ASSERT_NE(val, 3.14);
  ASSERT_NE(val, json::string_type{ "Hello World!" });

  second = json::newArray();
  second.push(1);
  second.push(2);
  second.push(3);
  second.push(4);
  int sum = 0;
  for (const json::Value & i : second)
    sum += i.toInteger();
  ASSERT_EQ(sum, 10);
  ASSERT_EQ(second.back(), 4);
  ASSERT_EQ(second.front(), 1);
}


TEST(jsontest, objects)
{
  json::Value val = json::newObject();
  val["two"] = 2;
  val["truth"] = false;

  ASSERT_EQ(val["two"], 2);
  ASSERT_FALSE(val["truth"].toBool());

  json::Object obj = val.toObject();
}

TEST(jsontest, serialization)
{
  json::Value val = json::write(true);
  ASSERT_TRUE(val.type() == json::ValueType::Boolean);
  bool b = json::read<bool>(val);
  ASSERT_TRUE(b);

  Point pt{ 1, 2 };
  val = json::write(pt);
  ASSERT_TRUE(val.type() == json::ValueType::Object);
  pt.x = pt.y = 0;
  pt = json::read<Point>(val);
  ASSERT_EQ(pt.x, 1);
}