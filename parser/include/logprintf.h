#include <iostream>
#include <string>
#include <chrono>
#include <ctime>

// 日志级别
enum class LogLevel {
    INFO,
    WARN,
    ERROR,
    DEBUG
};

// 获取当前时间字符串 [YYYY-MM-DD HH:MM:SS]
inline std::string get_current_time() {
    auto now = std::chrono::system_clock::now();
    std::time_t now_time = std::chrono::system_clock::to_time_t(now);
    char buf[32];
    std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", std::localtime(&now_time));
    return std::string(buf);
}

// 核心日志函数
inline void log(LogLevel level, const std::string& message) {
    // 输出时间
    std::cout << "[" << get_current_time() << "] ";

    // 输出级别
    switch (level) {
        case LogLevel::INFO:  std::cout << "[INFO]  "; break;
        case LogLevel::WARN:  std::cout << "[WARN]  "; break;
        case LogLevel::ERROR: std::cout << "[ERROR] "; break;
        case LogLevel::DEBUG: std::cout << "[DEBUG] "; break;
    }

    // 输出内容（支持任意类型，像 cout 一样用）
    (std::cout << message) << "\n";
}