#include <fstream>
#include <iostream>
#include "logger.hpp"
#include <ctime>

void Log::Init()
{
	char chLogBuff[128];
	char timestamp_buff[30];
	struct tm current_tm;
	time_t current_time = time(NULL);

	localtime_s(&current_tm, &current_time);
	sprintf_s(timestamp_buff, "[%02d/%02d/%d - %02d:%02d:%02d] %%s\n",
		current_tm.tm_mday, current_tm.tm_mon + 1, current_tm.tm_year + 1900,
		current_tm.tm_hour, current_tm.tm_min, current_tm.tm_sec);
	sprintf_s(chLogBuff, timestamp_buff, "sampnode plugin started...");

	std::ofstream file("sampnode.log", std::ofstream::out | std::ofstream::app);
	if (file.is_open())
	{
		file << chLogBuff;
		file.close();
	}
}

std::ostringstream& Log::Get(LogLevel level)
{
	time_t curr_time;
	curr_time = time(NULL);
	struct tm tm_local;
	localtime_s(&tm_local, &curr_time);

	char month_buff[12];
	char time_buff[12];
	sprintf_s(month_buff, "%02d/%02d/%d", tm_local.tm_mday, tm_local.tm_mon + 1, tm_local.tm_year + 1900);
	sprintf_s(time_buff, "%02d:%02d:%02d", tm_local.tm_hour, tm_local.tm_min, tm_local.tm_sec);
	os << "[" << month_buff << " - " << time_buff << "]";
	os << " -> " << GetLevelName(level) << ": ";
	return os;
}

Log::Log()
{

}

Log::~Log()
{
	os << std::endl;

	std::ofstream file("sampnode.log", std::ofstream::out | std::ofstream::app);
	if (file.is_open())
	{
		file << os.str();
		std::cout << os.str();
		os.str("");
		os.clear();
	}

}

std::string Log::GetLevelName(LogLevel level)
{
	std::string levelName = "[LOG_UNKNOWN]";
	switch (level)
	{
	case LogLevel::LOG_FATAL:
		levelName = "[FATAL ERROR]";
		break;
	case LogLevel::LOG_ERROR:
		levelName = "[ERROR]";
		break;
	case LogLevel::LOG_WARN:
		levelName = "[WARNING]";
		break;
	case LogLevel::LOG_INFO:
		levelName = "[INFO]";
		break;
	case LogLevel::LOG_DEBUG:
		levelName = "[DEBUG]";
		break;
	}
	return levelName;
}