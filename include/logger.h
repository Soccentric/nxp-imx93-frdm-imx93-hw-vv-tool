/**
 * @file logger.h
 * @brief Logging utility for the FRDM-IMX93 peripheral testing framework.
 * @author Sandesh Ghimire
 * @copyright (C) Soccentric LLC. All rights reserved.
 *
 * This header defines a singleton Logger class that provides thread-safe
 * logging capabilities with support for console output, file output,
 * and configurable log levels.
 *
 * @version 1.0
 * @date 2025-11-17
 */

#ifndef LOGGER_H
#define LOGGER_H

#include <chrono>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <memory>
#include <mutex>
#include <sstream>
#include <string>

namespace imx93_peripheral_test {

/**
 * @enum LogLevel
 * @brief Enumeration of available logging levels.
 *
 * Defines the severity levels for log messages, from most verbose
 * to most critical.
 */
enum class LogLevel { DEBUG, INFO, WARNING, ERROR };

/**
 * @class Logger
 * @brief Singleton logger class for thread-safe logging operations.
 *
 * Provides a centralized logging facility that can output to console,
 * files, or both. Supports different log levels and includes timestamps
 * in all log entries.
 *
 * @note This class is implemented as a singleton to ensure consistent
 *       logging behavior across the application.
 * @note All logging operations are thread-safe.
 */
class Logger {
public:
  /**
   * @brief Gets the singleton instance of the logger.
   *
   * Returns a reference to the single Logger instance. Thread-safe
   * due to static local variable initialization.
   *
   * @return Reference to the singleton Logger instance.
   */
  static Logger& instance() {
    static Logger instance;
    return instance;
  }

  /**
   * @brief Sets the output log file for file logging.
   *
   * Opens or changes the log file for writing log entries. If a file
   * was previously open, it is closed first. Log entries are appended
   * to the file.
   *
   * @param filename The path to the log file.
   *
   * @note Thread-safe operation.
   * @note Creates parent directories if they don't exist.
   */
  void set_log_file(const std::string& filename) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (file_stream_.is_open()) {
      file_stream_.close();
    }
    file_stream_.open(filename, std::ios::app);
  }

  /**
   * @brief Sets the minimum log level for output.
   *
   * Messages below this level will be ignored. Default is INFO.
   *
   * @param level The minimum log level to output.
   */
  void set_level(LogLevel level) {
    level_ = level;
  }

  /**
   * @brief Enables or disables console output.
   *
   * When disabled, log messages will only be written to file (if set).
   * Default is enabled.
   *
   * @param enable true to enable console output, false to disable.
   *
   * @note Thread-safe operation.
   */
  void set_console_output(bool enable) {
    std::lock_guard<std::mutex> lock(mutex_);
    console_output_ = enable;
  }

  /**
   * @brief Logs a message with the specified level.
   *
   * Formats and outputs a log message with timestamp, level indicator,
   * and the message. Outputs to console and/or file based on configuration.
   *
   * @param level The severity level of the message.
   * @param message The log message to output.
   *
   * @note Thread-safe operation.
   * @note ERROR level messages are sent to stderr, others to stdout.
   */
  void log(LogLevel level, const std::string& message) {
    if (level < level_)
      return;

    std::lock_guard<std::mutex> lock(mutex_);
    auto                        now  = std::chrono::system_clock::now();
    auto                        time = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;

    std::stringstream ss;
    ss << "[" << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S") << "."
       << std::setfill('0') << std::setw(3) << ms.count() << "] "
       << "[" << level_to_string(level) << "] " << message << "\n";

    std::string log_entry = ss.str();

    // Print to console if enabled
    if (console_output_) {
      if (level == LogLevel::ERROR) {
        std::cerr << log_entry;
      } else {
        std::cout << log_entry;
      }
    }

    // Write to file if open
    if (file_stream_.is_open()) {
      file_stream_ << log_entry;
      file_stream_.flush();
    }
  }

private:
  /**
   * @brief Private constructor for singleton pattern.
   *
   * Initializes the logger with default settings: INFO level, console output enabled.
   */
  Logger() : level_(LogLevel::INFO), console_output_(true) {}

  /**
   * @brief Private destructor.
   *
   * Ensures the log file is properly closed when the logger is destroyed.
   */
  ~Logger() {
    if (file_stream_.is_open()) {
      file_stream_.close();
    }
  }

  // Disable copy and assignment
  Logger(const Logger&)            = delete;
  Logger& operator=(const Logger&) = delete;

  /**
   * @brief Converts a LogLevel enum to its string representation.
   *
   * @param level The log level to convert.
   * @return String representation of the log level.
   */
  std::string level_to_string(LogLevel level) {
    switch (level) {
      case LogLevel::DEBUG:
        return "DEBUG";
      case LogLevel::INFO:
        return "INFO";
      case LogLevel::WARNING:
        return "WARN";
      case LogLevel::ERROR:
        return "ERROR";
      default:
        return "UNKNOWN";
    }
  }

  std::mutex    mutex_;         /**< Mutex for thread-safe operations */
  std::ofstream file_stream_;   /**< File stream for log file output */
  LogLevel      level_;         /**< Current minimum log level */
  bool          console_output_; /**< Whether to output to console */
};

// Helper macros
/**
 * @def LOG_DEBUG(msg)
 * @brief Logs a debug message.
 * @param msg The message to log.
 */
#define LOG_DEBUG(msg) \
  imx93_peripheral_test::Logger::instance().log(imx93_peripheral_test::LogLevel::DEBUG, msg)

/**
 * @def LOG_INFO(msg)
 * @brief Logs an info message.
 * @param msg The message to log.
 */
#define LOG_INFO(msg) \
  imx93_peripheral_test::Logger::instance().log(imx93_peripheral_test::LogLevel::INFO, msg)

/**
 * @def LOG_WARN(msg)
 * @brief Logs a warning message.
 * @param msg The message to log.
 */
#define LOG_WARN(msg) \
  imx93_peripheral_test::Logger::instance().log(imx93_peripheral_test::LogLevel::WARNING, msg)

/**
 * @def LOG_ERROR(msg)
 * @brief Logs an error message.
 * @param msg The message to log.
 */
#define LOG_ERROR(msg) \
  imx93_peripheral_test::Logger::instance().log(imx93_peripheral_test::LogLevel::ERROR, msg)

}  // namespace imx93_peripheral_test

#endif  // LOGGER_H
