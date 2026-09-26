#pragma once
#include <cstdio>
#include <ctime>
#include <string>

enum LogLevel
{
    LOG_INFO,
    LOG_WARN,
    LOG_ERROR
};

std::string get_time_str();

void Log(LogLevel lv, const char *fmt, ...);