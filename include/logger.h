#ifndef LOGGER_H
#define LOGGER_H

#include <string>
#include <iostream>
#include <fstream>
#include <mutex>
#include <memory>
#include <sstream>
#include <chrono>
#include <iomanip>

namespace imx93_peripheral_test {

enum class LogLevel {
    DEBUG,
    INFO,
    WARNING,
    ERROR
};

class Logger {
public:
    static Logger& instance() {
        static Logger instance;
        return instance;
    }

    void set_log_file(const std::string& filename) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (file_stream_.is_open()) {
            file_stream_.close();
        }
        file_stream_.open(filename, std::ios::app);
    }

    void set_level(LogLevel level) {
        level_ = level;
    }

    void set_console_output(bool enable) {
        std::lock_guard<std::mutex> lock(mutex_);
        console_output_ = enable;
    }

    void log(LogLevel level, const std::string& message) {
        if (level < level_) return;

        std::lock_guard<std::mutex> lock(mutex_);
        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;

        std::stringstream ss;
        ss << "[" << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S")
           << "." << std::setfill('0') << std::setw(3) << ms.count() << "] "
           << "[" << level_to_string(level) << "] "
           << message << "\n";

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
    Logger() : level_(LogLevel::INFO), console_output_(true) {}
    ~Logger() {
        if (file_stream_.is_open()) {
            file_stream_.close();
        }
    }
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    std::string level_to_string(LogLevel level) {
        switch (level) {
            case LogLevel::DEBUG: return "DEBUG";
            case LogLevel::INFO: return "INFO";
            case LogLevel::WARNING: return "WARN";
            case LogLevel::ERROR: return "ERROR";
            default: return "UNKNOWN";
        }
    }

    std::mutex mutex_;
    std::ofstream file_stream_;
    LogLevel level_;
    bool console_output_;
};

// Helper macros
#define LOG_DEBUG(msg) imx93_peripheral_test::Logger::instance().log(imx93_peripheral_test::LogLevel::DEBUG, msg)
#define LOG_INFO(msg) imx93_peripheral_test::Logger::instance().log(imx93_peripheral_test::LogLevel::INFO, msg)
#define LOG_WARN(msg) imx93_peripheral_test::Logger::instance().log(imx93_peripheral_test::LogLevel::WARNING, msg)
#define LOG_ERROR(msg) imx93_peripheral_test::Logger::instance().log(imx93_peripheral_test::LogLevel::ERROR, msg)

} // namespace imx93_peripheral_test

#endif // LOGGER_H
