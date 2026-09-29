#include "kvstore/util/logger.h"
#include <cstdarg>
#include <chrono>

static std::mutex log_mtx;
static std::ofstream log_file("kvstore.log", std::ios::app);
static bool log_enabled = true;
void SetLogEnabled(bool on) { log_enabled = on; }

std::string get_time_str()
{
    auto now = std::chrono::system_clock::now();
    auto total_ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
    time_t sec = total_ms / 1000;
    struct tm t;
    localtime_r(&sec, &t);
    char buf[32];
    strftime(buf, sizeof(buf), "[%Y-%m-%d %H:%M:%S", &t);
    char final_buf[40];
    snprintf(final_buf, sizeof(final_buf), "%s.%03ld]", buf, total_ms % 1000);
    return std::string(final_buf);
}

void Log(LogLevel lv, const char *fmt, ...)
{
    if (!log_enabled)
        return;
    std::lock_guard<std::mutex> lk(log_mtx);

    const char *level_str = "";
    switch (lv)
    {
    case LOG_INFO:
        level_str = "[INFO]";
        break;
    case LOG_ERROR:
        level_str = "[ERROR]";
        break;
    default:
        level_str = "[UNKNOWN]";
    }

    va_list ap;
    va_start(ap, fmt);
    char buf[1024];
    vsnprintf(buf, sizeof(buf), fmt, ap);
    va_end(ap);
    std::string line = get_time_str() + " " + level_str + " " + buf + '\n';
    printf("%s", line.c_str());
    fflush(stdout);
    log_file << line << std::flush;
}