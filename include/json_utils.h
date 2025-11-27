/**
 * @file json_utils.h
 * @brief Utility functions for JSON serialization and formatting.
 * @author Sandesh Ghimire
 * @copyright (C) Soccentric LLC. All rights reserved.
 *
 * This header provides helper functions for converting C++ data types
 * to JSON-formatted strings, including proper string escaping and
 * type-specific formatting.
 *
 * @version 1.0
 * @date 2025-11-17
 */

#ifndef JSON_UTILS_H
#define JSON_UTILS_H

#include <iomanip>
#include <map>
#include <sstream>
#include <string>
#include <vector>

namespace imx93_peripheral_test {

/**
 * @class JsonWriter
 * @brief Static utility class for JSON serialization.
 *
 * Provides static methods to convert various C++ data types to their
 * JSON string representations, with proper escaping for strings.
 */
class JsonWriter {
public:
  /**
   * @brief Escapes special characters in a string for JSON output.
   *
   * Converts a C++ string to a JSON-escaped string, handling quotes,
   * backslashes, control characters, and other special JSON characters.
   *
   * @param str The input string to escape.
   * @return A JSON-escaped string enclosed in double quotes.
   *
   * @note Follows JSON string escaping rules as per RFC 8259.
   */
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

  /**
   * @brief Template function to convert generic types to JSON values.
   *
   * Converts numeric and other types to their string representation.
   * This is the generic template that handles most arithmetic types.
   *
   * @tparam T The type of the value to convert.
   * @param value The value to convert to JSON format.
   * @return A string representation of the value.
   *
   * @note For strings, use the specialized version.
   */
  template <typename T>
  static std::string to_json_value(const T& value) {
    return std::to_string(value);
  }

  /**
   * @brief Specialization for string type conversion to JSON.
   *
   * Converts a C++ string to a properly escaped JSON string value.
   *
   * @param value The string value to convert.
   * @return A JSON-escaped string enclosed in quotes.
   */
  static std::string to_json_value(const std::string& value) {
    return escape_string(value);
  }

  /**
   * @brief Converts a boolean value to JSON format.
   *
   * @param value The boolean value to convert.
   * @return "true" or "false" as a JSON boolean value.
   */
  static std::string to_json_value(bool value) {
    return value ? "true" : "false";
  }
};

}  // namespace imx93_peripheral_test

#endif  // JSON_UTILS_H
