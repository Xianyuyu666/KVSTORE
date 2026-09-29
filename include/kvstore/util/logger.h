#pragma once
#include <cstdio>
#include <ctime>
#include <string>
#include <mutex>
#include <fstream>

enum LogLevel
{
    LOG_INFO,
    LOG_ERROR
};


std::string get_time_str();
void SetLogEnabled(bool on);
void Log(LogLevel lv, const char *fmt, ...);