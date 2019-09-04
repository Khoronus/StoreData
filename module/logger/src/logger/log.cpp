#include "logger/inc/logger/log.hpp"

namespace CmnLib
{
namespace control
{

//-- Begin of Logger rountine --------------------------------------------/
// Creates a Logger intance writing messages into STDOUT.
LoggerMS::LoggerMS(LogLevel level)
{
	level_ = level;
	file_ = nullptr;
}

// Creates a Logger instance writing messages into both STDOUT and log file.
LoggerMS::LoggerMS(std::string filename, LogLevel level)
{
	level_ = level;
	file_ = nullptr;
	ResetLogFile(filename);
}

LoggerMS::~LoggerMS()
{
	CloseLogFile();
}

int LoggerMS::ResetLogFile(std::string filename)
{
	CloseLogFile();
	if (filename.size() > 0) // try to open the log file if it is specified
	{
		file_ = fopen(filename.c_str(), "w");
		if (file_ == nullptr)
		{
			Error("Cannot create log file %s\n", filename.c_str());
			return -1;
		}
	}
	return 0;
}

void LoggerMS::Write(LogLevel level, const char *format, ...)
{
	va_list val;
	va_start(val, format);
	Write(level, format, val);
	va_end(val);
}

void LoggerMS::Debug(const char *format, ...)
{
	va_list val;
	va_start(val, format);
	Write(LogLevel::Debug, format, val);
	va_end(val);
}

void LoggerMS::Info(const char *format, ...)
{
	va_list val;
	va_start(val, format);
	Write(LogLevel::Info, format, val);
	va_end(val);
}

void LoggerMS::Error(const char *format, ...)
{
	va_list val;
	va_start(val, format);
	Write(LogLevel::Error, format, val);
	va_end(val);
}

void LoggerMS::Fatal(const char *format, ...)
{
	va_list val;
	va_start(val, format);
	Write(LogLevel::Fatal, format, val);
	va_end(val);
}

// TODO and DISCUSSION (thread safety issue): Two printing methods are 
// called for writing a message: one for printing the header information 
// and the other for printing the content. When multiple threads call the
// method simultaneously, there may be missed-order problem.
inline void LoggerMS::Write(LogLevel level, const char *format, va_list &val)
{
	if (level >= level_) // omit the message with low level
	{
		std::string level_str = GetLevelStr(level);
		std::string time_str = GetSystemTime();
		va_list val_copy;
		va_copy(val_copy, val);
		// write to STDOUT
		printf("[%s] [%s] ", level_str.c_str(), time_str.c_str());
		vprintf(format, val);
		fflush(stdout);
		// write to log file
		if (file_ != nullptr)
		{
			fprintf(file_, "[%s] [%s] ", level_str.c_str(), time_str.c_str());
			vfprintf(file_, format, val_copy);
			fflush(file_);
		}
		va_end(val_copy);

		if (is_kill_fatal_ && level == LogLevel::Fatal)
		{
			CloseLogFile();
			exit(1);
		}
	}
}

// Closes the log file if it it not null.
void LoggerMS::CloseLogFile()
{
	if (file_ != nullptr)
	{
		fclose(file_);
		file_ = nullptr;
	}
}

std::string LoggerMS::GetSystemTime()
{
	time_t t = time(0);
	char str[64];
	strftime(str, sizeof(str), "%Y-%m-%d %H:%M:%S", localtime(&t));
	return str;
}

std::string LoggerMS::GetLevelStr(LogLevel level)
{
	switch (level)
	{
	case LogLevel::Debug: return "DEBUG";
	case LogLevel::Info: return "INFO";
	case LogLevel::Error: return "ERROR";
	case LogLevel::Fatal: return "FATAL";
	default: return "UNKNOW";
	}
}
//-- End of Logger rountine ----------------------------------------------/

// Begin of Log class rountine -------------------------------------------/
LoggerMS LogMS::logger_;    // global (in process) static Logger instance

int LogMS::ResetLogFile(std::string filename)
{
	return logger_.ResetLogFile(filename);
}

void LogMS::ResetLogLevel(LogLevel level)
{
	logger_.ResetLogLevel(level);
}

void LogMS::ResetKillFatal(bool is_kill_fatal)
{
	logger_.ResetKillFatal(is_kill_fatal);
}

void LogMS::Write(LogLevel level, const char *format, ...)
{
	va_list val;
	va_start(val, format);
	logger_.Write(level, format, val);
	va_end(val);
}

void LogMS::Debug(const char *format, ...)
{
	va_list val;
	va_start(val, format);
	logger_.Write(LogLevel::Debug, format, val);
	va_end(val);
}

void LogMS::Info(const char *format, ...)
{
	va_list val;
	va_start(val, format);
	logger_.Write(LogLevel::Info, format, val);
	va_end(val);
}

void LogMS::Error(const char *format, ...)
{
	va_list val;
	va_start(val, format);
	logger_.Write(LogLevel::Error, format, val);
	va_end(val);
}

void LogMS::Fatal(const char *format, ...)
{
	va_list val;
	va_start(val, format);
	logger_.Write(LogLevel::Fatal, format, val);
	va_end(val);
}
// End of Log class rountine ---------------------------------------------/


} // namespace CmnLib
} //namespace control

