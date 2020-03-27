#pragma once
#include <sstream>

#define L_INFO Log().Get(LogLevel::LOG_INFO)
#define L_DEBUG Log().Get(LogLevel::LOG_DEBUG)
#define L_WARN Log().Get(LogLevel::LOG_WARN)
#define L_ERROR Log().Get(LogLevel::LOG_ERROR)

enum class LogLevel
{
	LOG_ERROR = 0, // level 1
	LOG_WARN = 1, // level 2
	LOG_DEBUG = 2, // level 3
	LOG_INFO = 3 // level 4
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
	LogLevel currentLevel;

	std::string GetLevelName(LogLevel messageLevel);
	Log(const Log&);
};