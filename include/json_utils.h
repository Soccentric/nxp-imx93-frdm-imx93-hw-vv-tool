#ifndef JSON_UTILS_H
#define JSON_UTILS_H

#include <iomanip>
#include <map>
#include <sstream>
#include <string>
#include <vector>

namespace imx93_peripheral_test {

class JsonWriter {
public:
  static std::string escape_string(const std::string& str) {
    std::stringstream ss;
    ss << "\"";
    for (char c : str) {
      switch (c) {
        case '"':
          ss << "\\\"";
          break;
        case '\\':
          ss << "\\\\";
          break;
        case '\b':
          ss << "\\b";
          break;
        case '\f':
          ss << "\\f";
          break;
        case '\n':
          ss << "\\n";
          break;
        case '\r':
          ss << "\\r";
          break;
        case '\t':
          ss << "\\t";
          break;
        default:
          if (static_cast<unsigned char>(c) <= '\x1f') {
            ss << "\\u" << std::hex << std::setw(4) << std::setfill('0')
               << static_cast<unsigned char>(c);
          } else {
            ss << c;
          }
      }
    }
    ss << "\"";
    return ss.str();
  }

  template <typename T>
  static std::string to_json_value(const T& value) {
    return std::to_string(value);
  }

  static std::string to_json_value(const std::string& value) {
    return escape_string(value);
  }

  static std::string to_json_value(bool value) {
    return value ? "true" : "false";
  }
};

}  // namespace imx93_peripheral_test

#endif  // JSON_UTILS_H
