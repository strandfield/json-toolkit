// Copyright (C) 2019 Vincent Chambrin
// This file is part of the json-toolkit library
// For conditions of distribution and use, see copyright notice in LICENSE

#include <map>
#include <string>
#include <sstream>
#include <vector>

#include "json-toolkit/json-global-defs.h"

#define JSON_HAS_DEFAULT_PARSER_BACKEND
#define JSON_HAS_DEFAULT_WRITER_BACKEND

namespace json
{

namespace config
{

using integer_type = int;
using number_type = double;
using string_type = std::string;

template<typename T>
using array_type = std::vector<T>;

template<typename Key, typename T>
using map_type = std::map<Key, T>;

inline int string_compare(const string_type& lhs, const string_type& rhs)
{
  return lhs.compare(rhs);
}

} // namespace config

} // namespace json

