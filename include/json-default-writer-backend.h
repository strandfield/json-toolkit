
namespace json
{

namespace config
{

struct DefaultWriterBackend
{
  std::stringstream result_;

  std::string result() { return result_.str(); }

  DefaultWriterBackend& operator<<(CharCategory c)
  {
    switch (c)
    {
    case CharCategory::Space:
      result_ << " ";
      break;
    case CharCategory::NewLine:
      result_ << "\n";
      break;
    case CharCategory::LBrace:
      result_ << "{";
      break;
    case CharCategory::RBrace:
      result_ << "}";
      break;
    case CharCategory::LBracket:
      result_ << "[";
      break;
    case CharCategory::RBracket:
      result_ << "]";
      break;
    case CharCategory::Colon:
      result_ << ":";
      break;
    case CharCategory::Comma:
      result_ << ",";
      break;
    case CharCategory::SingleQuote:
      result_ << "'";
      break;
    case CharCategory::DoubleQuote:
      result_ << "\"";
      break;
    }

    return *this;
  }

  DefaultWriterBackend& operator<<(nullptr_t)
  {
    result_ << "null";
    return *this;
  }

  DefaultWriterBackend& operator<<(bool value)
  {
    result_ << (value ? "true" : "false");
    return *this;
  }

  DefaultWriterBackend & operator<<(int value)
  {
    result_ << value;
    return *this;
  }

  DefaultWriterBackend& operator<<(config::number_type value)
  {
    result_ << value;
    return *this;
  }

  DefaultWriterBackend& operator<<(const config::string_type & str)
  {
    result_ << str;
    return *this;
  }
};

} // namespace config

} // namespace json
