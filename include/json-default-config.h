
#include <map>
#include <string>
#include <vector>

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
