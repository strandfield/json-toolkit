
#include <gtest/gtest.h>

#include "json.h"
#include "serialization.h"

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

struct Point
{
  int x; 
  int y;
};

namespace json
{

namespace serialization
{

template<>
struct decoder<Point>
{
  static void decode(Serializer& s, const Json& data, Point& pt)
  {
    pt.x = data["x"].toInt();
    pt.y = data["y"].toInt();
  }
};

template<>
struct encoder<Point>
{
  static Json encode(Serializer& s, const Point& pt)
  {
    Json result = {};
    result["x"] = pt.x;
    result["y"] = pt.y;
    return result;
  }
};

} // namespace serialization

} // namespace json

TEST(jsontest, templateSerialization)
{
  using namespace json;

  Serializer s;

  {
    Point pt{ 1, 2 };
    Json data = s.encode(pt);

    ASSERT_EQ(data["x"], 1);
    ASSERT_EQ(data["y"], 2);

    data["x"] = 4;
    pt = s.decode<Point>(data);
    ASSERT_EQ(pt.x, 4);
  }

  {
    config::array_type<Point> pts;
    pts.push_back(Point{ 1, 2 });
    pts.push_back(Point{ 3, 4 });

    Json data = s.encode(pts);

    ASSERT_EQ(data.length(), 2);

    ASSERT_TRUE(data[0]["x"] == 1);
    ASSERT_TRUE(data[1]["y"] == 4);

    data[1]["y"] = 1;

    pts = s.decode<decltype(pts)>(data);
    ASSERT_EQ(pts.size(), 2);
    ASSERT_EQ(pts.back().y, 1);
  }
}

TEST(jsontest, codecSerialization)
{
  using namespace json;

  Serializer s;

  {
    auto* codec = ObjectCodec::create<Point>();
    codec->addField("xx", &Point::x);
    codec->addField("yy", &Point::y);
    s.addCodec(codec);
  }

  {
    Point pt{ 1, 2 };
    Json data = s.encode(pt);

    ASSERT_EQ(data["xx"], 1);
    ASSERT_EQ(data["yy"], 2);

    data["xx"] = 4;
    pt = s.decode<Point>(data);
    ASSERT_EQ(pt.x, 4);
  }
}
