/**
* @file logger_defines.hpp
* @brief Header to call all the control headers.
*
* @section LICENSE
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
* ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHOR/AUTHORS BE LIABLE FOR ANY
* DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
* (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
* LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
* ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
* THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*
* @author  Microsoft
* @bug No known bugs.
* @version 1.0.0.0
* @link https://github.com/Microsoft/multiverso
*
*/

#ifndef CMNLIB_CONTROL_LOG_HPP__
#define CMNLIB_CONTROL_LOG_HPP__

#include <fstream>
#include <string>
#include <ctime>
#include <cstdarg>

#include "logger_defines.hpp"

namespace CmnLib
{
namespace control
{

/*!
    * \brief A enumeration type of log message levels. The values are ordered:
    *        Debug < Info < Error < Fatal.
    */
enum class STOREDATA_LOGGER_EXPORT LogLevel : int
{
    Debug = 0,
    Info = 1,
    Error = 2,
    Fatal = 3
};

/*!
    * \brief The Logger class is responsible for writing log messages into
    *        standard output or log file.
    */
class STOREDATA_LOGGER_EXPORT LoggerMS
{
    // Enable the static Log class to call the private method.
	friend class LogMS;

public:
    /*! 
        * \brief Creates an instance of Logger class. By default, the log 
        *        messages will be written to standard output with minimal 
        *        level of INFO. Users are able to further set the log file or
        *        log level with corresponding methods.
        * \param level Minimal log level, Info by default.
        */
	explicit LoggerMS(LogLevel level = LogLevel::Info);
    /*! 
        * \brief Creates an instance of Logger class by specifying log file
        *        and log level. The log message will be written to both STDOUT
        *        and file (if created successfully).
        * \param filename Log file name
        * \param level Minimal log level
        */
	explicit LoggerMS(std::string filename, LogLevel level = LogLevel::Info);
	~LoggerMS();

    /*! 
        * \brief Resets the log file. 
        * \param filename The new log filename. If it is empty, the Logger
        *        will close current log file (if it exists).
        * \return Returns -1 if the filename is not empty but failed on 
        *         creating the log file, or 0 will be returned otherwise.
        */
    int ResetLogFile(std::string filename);
    /*! 
        * \brief Resets the log level.
        * \param level The new log level.
        */
    void ResetLogLevel(LogLevel level) { level_ = level; }
    /*!
        * \brief Resets the option of whether kill the process when fatal 
        *        error occurs. By defualt the option is false.
        */
    void ResetKillFatal(bool is_kill_fatal) { is_kill_fatal_ = is_kill_fatal; }

    /*!
        * \brief C style formatted method for writing log messages. A message
        *        is with the following format: [LEVEL] [TIME] message
        * \param level The log level of this message.
        * \param format The C format string.
        * \param ... Output items.
        */
    void Write(LogLevel level, const char *format, ...);
    void Debug(const char *format, ...);
    void Info(const char *format, ...);
    void Error(const char *format, ...);
    void Fatal(const char *format, ...);

private:
    void Write(LogLevel level, const char *format, va_list &val);
    void CloseLogFile();
    // Returns current system time as a string.
    std::string GetSystemTime();
    // Returns the string of a log level.
    std::string GetLevelStr(LogLevel level);

    std::FILE *file_; // A file pointer to the log file.
    LogLevel level_;  // Only the message not less than level_ will be outputed.
    bool is_kill_fatal_; // If kill the process when fatal error occurs.

    // No copying allowed
	LoggerMS(const LoggerMS&);
	void operator=(const LoggerMS&);
};

/*! 
    * \brief The Log class is a static wrapper of a global Logger instance in 
    *        the scope of a process. Users can write logging messages easily
    *        with the static methods.
    */
class STOREDATA_LOGGER_EXPORT LogMS
{
public:
    /*!
        * \brief Resets the log file. The logger will write messages to the 
        *        log file if it exists in addition to the STDOUT by default.
        * \param filename The log filename. If it is empty, the logger will 
        *        close the current log file (if it exists) and only output to
        *        STDOUT.
        * \return -1 if fail on creating the log file, or 0 otherwise.
        */
    static int ResetLogFile(std::string filename);
    /*!
        * \brief Resets the minimal log level. It is INFO by default.
        * \param level The new minimal log level.
        */
    static void ResetLogLevel(LogLevel level);
    /*!
    * \brief Resets the option of whether kill the process when fatal
    *        error occurs. By defualt the option is false.
    */
    static void ResetKillFatal(bool is_kill_fatal);

    /*! \brief The C formatted methods of writing the messages. */
    static void Write(LogLevel level, const char *format, ...);
    static void Debug(const char *format, ...);
    static void Info(const char *format, ...);
    static void Error(const char *format, ...);
    static void Fatal(const char *format, ...);

private:
	static LoggerMS logger_;
};

}  // namespace control
}  // namespace CmnLib


#endif /* CMNLIB_CONTROL_LOG_HPP__ */
