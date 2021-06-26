

# `json-toolkit`

`json-toolkit` is header-only library providing several Json utilities, it is released under the MIT license.

[![Build Status](https://api.travis-ci.com/strandfield/json-toolkit.svg?branch=master)](https://travis-ci.com/github/strandfield/json-toolkit)

## Documentation

### Json objects

```cpp
#include "json-toolkit/json.h"
```

This header provides 3 classes to create and manipulate Json objects:
- Json
- Array
- Object

Most operations (including those related to objects or arrays) can be done with the `Json` class. The `Array` and `Object` class can be used to have access to the underlying C++ container (i.e. `std::vector` or `std::map`).

The library supports 5 fundamentals data-type (null, boolean, integer, number and string) as shown below.

```cpp
Json obj = {};
obj["bool"] = true;
obj["string"] = "Hello World!";
obj["integer"] = 42;
obj["number"] = 3.14;
obj["invalid"] = nullptr;
```

An empty array can be constructed using the `Array` class default constructor.

```cpp
Json vec = Array();
vec.push(1);
vec.push(2);
vec.push(3);
```

The `Array` and `Object` class allow access to the C++ containers.

```cpp
Json value = ...;
if(value.isOject())
{
  Object obj = value.toObject();
  std::map<std::string, Json>& map = obj.data();
}
else if(value.isArray())
{
  Array array = value.toArray();
  std::vector<Json>& vec = array.data();
}
```

Json objects can be compared for equality using `==` and `!=`.

### Serialization of C++ objects

```cpp
#include "json-toolkit/serialization.h"
```

This header provides utilities to convert C++ objects to and fro `Json`.
Everything is done through the `Serializer` class. The built-in types are supported out of the box.

```cpp
Serializer s;

{
  std::vector<int> vec{1, 2, 3, 4, 5};
  Json val = s.encode(vec);
  std::vector<int> decoded = s.decode<std::vector<int>>(val);
  assert(decoded == vec);
}

{
  std::variant<int, bool, std::string> alternative = ...;
  Json val = s.encode(alternative);
}
```

Two interfaces are provided to add encoding/decoding support for custom types.

The first one is by fully-specializing the class templates `encoder` and `decoder` in namespace `json::serialization` (see example in `tests.cpp`).

The second one is by defining a `Codec` for your type.

```cpp
struct Point {
  int x, y;
};

auto* codec = ObjectCodec::create<Point>();
codec->addField("x", &Point::x);
codec->addField("x", &Point::y);
serializer.addCodec(codec);
```

### Parsing

```cpp
#include "json-toolkit/parsing.h"
```

This header provides parsing utilities. The high-level `parse` will create a `Json` value from a string. 

```cpp
Json value = json::parse("[1, 2, 3]");
```

For more advanced use, a template class `ParserMachine` provides a state-machine parser with custom backend that can be used to process partial Json strings.

### Stringify

```cpp
#include "json-toolkit/stringify.h"
```

This header provides both a high-level function `stringify` and a template `GenericWriter` to write Json strings.

```cpp
Json obj = ...;
std::string str = json::stringify(obj);
```
