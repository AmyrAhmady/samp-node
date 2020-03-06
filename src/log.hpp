#pragma once
#include <sstream>

#define L_INFO Log().Get(LogLevel::LOG_INFO)
#define L_DEBUG Log().Get(LogLevel::LOG_DEBUG)
#define L_WARN Log().Get(LogLevel::LOG_WARN)
#define L_ERROR Log().Get(LogLevel::LOG_ERROR)
#define L_FATAL Log().Get(LogLevel::LOG_FATAL)

enum class LogLevel {
    LOG_FATAL, LOG_ERROR, LOG_WARN, LOG_INFO, LOG_DEBUG
};

class Log
{
public:
    static void Init();

    Log();
    virtual ~Log();
    std::ostringstream& Get(LogLevel level = LogLevel::LOG_INFO);

protected:
    std::ostringstream os;
private:
    std::string GetLevelName(LogLevel messageLevel);
    Log(const Log&);
};