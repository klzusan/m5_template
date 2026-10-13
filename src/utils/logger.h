#pragma once
#include <Arduino.h>

// ロガー機能 (シリアル出力用 ANSI カラー付きフォーマット)
enum class LogLevel {
    DEBUG,
    INFO,
    WARN,
    ERROR
};

class Logger {
public:
    static void init(unsigned long baud = 115200) {
        if (!Serial) {
            Serial.begin(baud);
        }
    }

    static void log(LogLevel level, const char* tag, const char* format, ...) {
        char buffer[256];
        va_list args;
        va_start(args, format);
        vsnprintf(buffer, sizeof(buffer), format, args);
        va_end(args);

        const char* colorCode = "\033[0m";
        const char* levelStr  = "INFO";

        switch (level) {
            case LogLevel::DEBUG:
                colorCode = "\033[36m"; // Cyan
                levelStr  = "DEBUG";
                break;
            case LogLevel::INFO:
                colorCode = "\033[32m"; // Green
                levelStr  = "INFO";
                break;
            case LogLevel::WARN:
                colorCode = "\033[33m"; // Yellow
                levelStr  = "WARN";
                break;
            case LogLevel::ERROR:
                colorCode = "\033[31m"; // Red
                levelStr  = "ERROR";
                break;
        }

        // [millis] [LEVEL] [TAG] message
        Serial.printf("%s[%8lu][%s][%s] %s\033[0m\r\n", 
                      colorCode, millis(), levelStr, tag, buffer);
    }
};

#define LOG_D(tag, ...) Logger::log(LogLevel::DEBUG, tag, __VA_ARGS__)
#define LOG_I(tag, ...) Logger::log(LogLevel::INFO,  tag, __VA_ARGS__)
#define LOG_W(tag, ...) Logger::log(LogLevel::WARN,  tag, __VA_ARGS__)
#define LOG_E(tag, ...) Logger::log(LogLevel::ERROR, tag, __VA_ARGS__)
