
#include <gtest/gtest.h>

#include "json.h"

TEST(jsontest, values)
{
  json::Json var = nullptr;
  ASSERT_TRUE(var.isNull());
  ASSERT_TRUE(var == nullptr);

  var = 5;
  ASSERT_TRUE(var.isInteger());
  ASSERT_EQ(var.toInt(), 5);
  ASSERT_TRUE(var == 5);
  ASSERT_FALSE(var == 6);
  ASSERT_FALSE(var == true);
  ASSERT_FALSE(var == nullptr);

  var = true;
  ASSERT_TRUE(var.isBoolean());

  var = 3.0;
  ASSERT_TRUE(var.isNumber());

  var = "Hello World";
  ASSERT_TRUE(var.isString());
}


TEST(jsontest, arrays)
{
  json::Array val = json::Array();

  ASSERT_TRUE(val.isArray());
  ASSERT_FALSE(val.toArray().isNull());

  val.push(true);
  val.push(2);
  ASSERT_EQ(val.length(), 2);

  ASSERT_EQ(val[0], true);
  val[0] = 5;
  ASSERT_EQ(val[0], 5);

  json::Array second = json::Array();
  second.push(5);
  second.push(2);
  ASSERT_EQ(second, val);
  second[1] = 3;
  ASSERT_NE(second, val);

  ASSERT_NE(val, true);
  ASSERT_NE(val, 5);
  ASSERT_NE(val, 3.14);
  ASSERT_NE(val, json::Json("Hello World!"));

  second = json::Array();
  second.push(1);
  second.push(2);
  second.push(3);
  second.push(4);
  int sum = 0;
  for (const json::Json & i : second.data())
    sum += i.toInt();
  ASSERT_EQ(sum, 10);
  ASSERT_EQ(second.data().back(), 4);
  ASSERT_EQ(second.data().front(), 1);
}


TEST(jsontest, objects)
{
  json::Json val = json::Object();

  ASSERT_TRUE(val.isObject());
  ASSERT_FALSE(val.toObject().isNull());

  val["two"] = 2;
  val["truth"] = false;

  ASSERT_EQ(val["two"], 2);
  ASSERT_FALSE(val["truth"].toBool());

  json::Json obj = {};
  obj["foo"]["bar"] = "Hello";

  ASSERT_EQ(obj["foo"]["bar"].toString(), "Hello");

  val = json::Object();
  val["foo"] = "bar";

  ASSERT_TRUE(obj != val);

  val = json::Array();
  ASSERT_TRUE(obj != val);
}
