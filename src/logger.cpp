#include "kvstore/util/logger.h"
#include <cstdarg>
#include <chrono>

std::string get_time_str()
{
    auto now = std::chrono::system_clock::now();
    auto total_ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
    time_t sec = total_ms / 1000;
    struct tm t;
    localtime_r(&sec,&t);
    char buf[32];
    strftime(buf,sizeof(buf),"[%Y-%m-%d %H:%M:%S",&t);
    char final_buf[40];
    snprintf(final_buf,sizeof(final_buf),"%s.%03ld]",buf,total_ms % 1000);
    return std::string(final_buf);
}

void Log(LogLevel lv, const char *fmt, ...)
{
    const char* level_str = "";
    switch(lv){
        case LOG_INFO: level_str = "[INFO]";break;
        case LOG_WARN: level_str = "[WARN]";break;
        case LOG_ERROR: level_str = "[ERROR]";break;
        default: level_str = "[UNKNOWN]";
    }

    va_list ap;
    va_start(ap,fmt);
    char buf[1024];
    vsnprintf(buf,sizeof(buf),fmt,ap);
    va_end(ap);

    printf("%s %s %s\n",get_time_str().c_str(),level_str,buf);
    fflush(stdout);
}